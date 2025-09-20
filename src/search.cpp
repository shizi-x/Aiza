#include "search.hpp"
#include "utils.hpp"
#include "thread_pool.hpp"

#include <iostream>
#include <atomic>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// PCRE2
#include <pcre2.h>

// JSON
#include <nlohmann/json.hpp>

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

static bool name_match(const string &name, const SearchOptions &opt) {
    if (opt.fuzzy) {
        int d = util::levenshtein(name, opt.pattern);
        return d <= opt.fuzzy_threshold;
    }
    // glob or substring fallback
    return util::matches_glob(name, opt.pattern, opt.icase);
}

// mmap helper: search a needle (bytes) inside file using memmem or PCRE2 if regex
static bool mmap_search_bytes(const fs::path &p, const char* needle, size_t needle_len) {
    int fd = open(p.c_str(), O_RDONLY);
    if (fd < 0) return false;
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return false; }
    if (st.st_size == 0) { close(fd); return false; }
    size_t len = (size_t)st.st_size;
    void* map = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) { close(fd); return false; }
    const void* found = util::memmem_portable(map, len, needle, needle_len);
    munmap(map, len);
    close(fd);
    return found != nullptr;
}

// For substring text search (not regex): use mmap and case-insensitive handling if needed
static void search_file_substr_mmap(const fs::path &p, const SearchOptions &opt, const ResultCallback &cb) {
    ifstream check(p);
    if (!check) return;
    if (util::looks_like_binary(p) && !opt.binary_search) return;

    int fd = open(p.c_str(), O_RDONLY);
    if (fd < 0) return;
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return; }
    if (st.st_size == 0) { close(fd); return; }
    size_t len = (size_t)st.st_size;
    void* map = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) { close(fd); return; }

    const char* data = (const char*)map;
    size_t needle_len = opt.pattern.size();
    if (opt.binary_search) {
        bool ok = util::memmem_portable(data, len, opt.pattern.data(), needle_len) != nullptr;
        if (ok) { Result r; r.path = p; cb(r); }
    } else {
        if (!opt.icase) {
            const void* found = util::memmem_portable(data, len, opt.pattern.data(), needle_len);
            if (found) {
                // Attempt to find the line number and content for nicer output:
                const char* pos = (const char*)found;
                // find line start
                const char* start = pos;
                while (start > data && *(start-1) != '\n') --start;
                const char* end = pos;
                while ((size_t)(end - data) < len && *end != '\n') ++end;
                string line(start, end - start);
                size_t line_no = 1;
                for (const char* t = data; t < start; ++t) if (*t == '\n') ++line_no;
                Result r; r.path = p; r.matched_line = line; r.line_no = line_no; cb(r);
            }
        } else {
            // case-insensitive: fallback to scanning by lines to handle character folding reliably
            // (could implement lowercasing the mmap region -- memory heavy)
            ifstream in(p);
            if (!in) { munmap(map,len); close(fd); return; }
            string line; size_t ln=0;
            string needle = opt.pattern;
            transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
            while (getline(in, line)) {
                ++ln;
                string t = line;
                transform(t.begin(), t.end(), t.begin(), ::tolower);
                if (t.find(needle) != string::npos) {
                    Result r; r.path = p; r.matched_line = line; r.line_no = ln; cb(r);
                }
            }
        }
    }
    munmap(map, len);
    close(fd);
}

// Search file with PCRE2 regex via mmap (we compile once per Searcher and pass the compiled pattern)
static void search_file_regex_mmap(const fs::path &p, const pcre2_code* re_code, const SearchOptions &opt, const ResultCallback &cb) {
    ifstream check(p);
    if (!check) return;
    if (util::looks_like_binary(p) && !opt.binary_search) return;

    int fd = open(p.c_str(), O_RDONLY);
    if (fd < 0) return;
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return; }
    if (st.st_size == 0) { close(fd); return; }
    size_t len = (size_t)st.st_size;
    void* map = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) { close(fd); return; }

    // PCRE2 works on UTF-8 / bytes equally; use pcre2_match on buffer chunks (but avoid splitting matches)
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re_code, NULL);
    PCRE2_SPTR subject = (PCRE2_SPTR)map;
    size_t subject_length = len;
    int rc = pcre2_match(re_code, subject, subject_length, 0, 0, match_data, NULL);
    if (rc >= 0) {
        // find the first match offset to extract a line
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        size_t start = (size_t)ovector[0];
        // compute line start and end
        const char* data = (const char*)map;
        const char* pos = data + start;
        const char* sstart = pos;
        while (sstart > data && *(sstart-1) != '\n') --sstart;
        const char* send = data + subject_length;
        const char* send2 = pos;
        while ((size_t)(send2 - data) < subject_length && *send2 != '\n') ++send2;
        string line(sstart, send2 - sstart);
        size_t line_no = 1;
        for (const char* t = data; t < sstart; ++t) if (*t == '\n') ++line_no;
        Result r; r.path = p; r.matched_line = line; r.line_no = line_no; cb(r);
    }
    pcre2_match_data_free(match_data);
    munmap(map, len);
    close(fd);
}

Searcher::Searcher(const SearchOptions &opt) : opt(opt) {}

void Searcher::run(const ResultCallback &cb) {
    ThreadPool pool(opt.threads ? opt.threads : 4);
    atomic<size_t> files_processed{0};
    atomic<size_t> total_seen{0};

    // compile PCRE2 regex if needed
    pcre2_code *re_code = nullptr;
    if (opt.content_regex && !opt.regex_pattern.empty()) {
        int errornumber;
        PCRE2_SIZE erroroffset;
        uint32_t options = 0;
        if (opt.icase) options |= PCRE2_CASELESS;
        re_code = pcre2_compile(
            (PCRE2_SPTR)opt.regex_pattern.c_str(),
            PCRE2_ZERO_TERMINATED,
            options,
            &errornumber,
            &erroroffset,
            NULL
        );
        if (re_code == NULL) {
            // compile error: fallback to not using regex
            cerr << "PCRE2 compilation failed at offset " << erroroffset << "\n";
            opt.content_regex = false;
        }
    }

    atomic<bool> done{false};
    thread progress([&]{
        while (!done) {
            size_t seen = total_seen.load();
            size_t donec = files_processed.load();
            cerr << "\rscanned: " << seen << " entries, processed files: " << donec << "    " << flush;
            this_thread::sleep_for(chrono::milliseconds(350));
        }
    });

    // iterate roots
    for (const auto &root : opt.roots) {
        fs::path r(root);
        if (!fs::exists(r)) continue;
        for (fs::recursive_directory_iterator it(r, fs::directory_options::skip_permission_denied), end; it != end; it.increment()) {
            auto entry = *it;
            total_seen++;
            fs::path p = entry.path();

            // check ignore rules relative to root
            if (opt.ignore.is_ignored(r, p)) {
                if (entry.is_directory()) it.disable_recursion_pending();
                continue;
            }

            string name = p.filename().string();

            // directories
            if (entry.is_directory()) {
                if (opt.find_dirs && name_match(name, opt)) {
                    Result res; res.path = p; res.is_dir = true; cb(res);
                }
                continue;
            }

            if (entry.is_regular_file()) {
                // name match
                if (opt.find_files && name_match(name, opt)) {
                    Result res; res.path = p; res.is_dir = false; cb(res);
                }

                // content search modes
                if (opt.content_substr) {
                    // submit a job to search substring using mmap
                    pool.enqueue([p, this, &cb, &files_processed]() {
                        search_file_substr_mmap(p, this->opt, cb);
                        files_processed++;
                    });
                } else if (opt.content_regex && re_code) {
                    pool.enqueue([p, re_code, this, &cb, &files_processed]() {
                        search_file_regex_mmap(p, re_code, this->opt, cb);
                        files_processed++;
                    });
                } else if (opt.content_regex && !re_code) {
                    // fallback: nothing
                }

                // rx combined: if opt.content_regex==false but user set rx-like behavior by setting both pattern & regex_pattern
                if (!opt.content_regex && !opt.regex_pattern.empty()) {
                    // name match already attempted; optionally search contents by substring of pattern
                    pool.enqueue([p, this, &cb, &files_processed]() {
                        search_file_substr_mmap(p, this->opt, cb);
                        files_processed++;
                    });
                }
            }
        }
    }

    // pool destructor will wait for tasks
    // wait a short time for queue to drain (ThreadPool destructor will handle joining)
    // mark done
    done = true;
    if (progress.joinable()) progress.join();

    if (re_code) pcre2_code_free(re_code);
}


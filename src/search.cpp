#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "search.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;

// Safe unused function
[[maybe_unused]] static bool mmap_search_bytes(const fs::path &p, const char* needle, size_t needle_len) {
    (void)p;
    (void)needle;
    (void)needle_len;
    return false;
}

[[maybe_unused]] static void search_file_regex_mmap(
    const fs::path &p,
    const pcre2_code* re_code,
    const SearchOptions &opt,
    const ResultCallback &cb
) {
    if (util::looks_like_binary(p) && !opt.content_substr) return;

    std::ifstream file(p, std::ios::binary);
    if (!file) return;

    std::string line;
    size_t line_no = 0;

    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern_8(re_code, nullptr);

    while (std::getline(file, line)) {
        ++line_no;
        PCRE2_SPTR subject = reinterpret_cast<PCRE2_SPTR>(line.c_str());
        size_t subject_length = line.size();
        int rc = pcre2_match_8(re_code, subject, subject_length, 0, 0, match_data, nullptr);

        if (rc >= 0) {
            Result r;
            r.path = p;
            r.matched_line = line;
            r.line_no = line_no;
            cb(r);
        }
    }

    pcre2_match_data_free_8(match_data);
}

// Constructor
Searcher::Searcher(const SearchOptions &options) : opt(options) {}

// Run function
void Searcher::run(const ResultCallback &cb) {
    pcre2_code *re_code = nullptr;
    int errornumber;
    PCRE2_SIZE erroroffset;

    if (!opt.regex_pattern.empty()) {
        re_code = pcre2_compile_8(
            reinterpret_cast<PCRE2_SPTR>(opt.regex_pattern.c_str()),
            opt.regex_pattern.size(),
            0,
            &errornumber,
            &erroroffset,
            nullptr
        );

        if (!re_code) {
            std::cerr << "Failed to compile regex at offset " << erroroffset << ", error " << errornumber << "\n";
            return;
        }
    }

    for (fs::recursive_directory_iterator it(opt.path, fs::directory_options::skip_permission_denied), end; it != end; ++it) {
        const fs::path &p = it->path();
        if (re_code)
            search_file_regex_mmap(p, re_code, this->opt, cb);
        else {
            // fallback: just return file path if no regex
            Result r;
            r.path = p;
            r.line_no = 0;
            r.matched_line = "";
            cb(r);
        }
    }

    if (re_code) pcre2_code_free_8(re_code);
}


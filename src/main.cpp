#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include "search.hpp"
#include "utils.hpp"
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

int main(int argc, char** argv) {
    SearchOptions opt;
    if (argc == 1) { cout << "Usage: aiza [options] <paths>\n"; return 0; }

    // basic parsing
    for (int i=1;i<argc;++i) {
        string a = argv[i];
        if (a=="-d" && i+1<argc) { opt.find_dirs = true; opt.pattern = argv[++i]; }
        else if (a=="-f" && i+1<argc) { opt.find_files = true; opt.pattern = argv[++i]; }
        else if (a=="-z" && i+1<argc) { opt.content_substr = true; opt.pattern = argv[++i]; }
        else if (a=="-R" && i+1<argc) { opt.content_regex = true; opt.regex_pattern = argv[++i]; }
        else if (a=="-rx" && i+1<argc) { opt.regex_pattern = argv[++i]; opt.pattern = argv[i]; }
        else if (a=="--json") opt.json = true;
        else if (a=="--json-array") opt.json_array = true;
        else if (a=="--no-color") opt.color = false;
        else if (a=="--binary") opt.binary_search = true;
        else if (a=="--fuzzy") opt.fuzzy = true;
        else if (a=="-i") opt.icase = true;
        else if (a=="-t" && i+1<argc) opt.threads = stoul(argv[++i]);
        else if (a=="--ignore-file" && i+1<argc) opt.ignore.load_from_file(argv[++i]);
        else opt.roots.push_back(a);
    }

    if (opt.roots.empty()) opt.roots.push_back(".");

    // result collection and output
    mutex outm;
    bool first_json = true;

    if (opt.json_array) cout << "[\n";

    Searcher s(opt);
    s.run([&](const Result &r) {
        lock_guard<mutex> lk(outm);
        if (opt.json || opt.json_array) {
            json j;
            j["path"] = r.path.string();
            j["is_dir"] = r.is_dir;
            if (!r.matched_line.empty()) {
                j["line"] = r.matched_line;
                j["line_no"] = r.line_no;
            }
            if (opt.json_array) {
                if (!first_json) cout << ",\n";
                cout << j.dump();
            } else {
                cout << j.dump() << "\n";
            }
            first_json = false;
        } else {
            if (!r.matched_line.empty()) {
                if (opt.color) {
                    cout << r.path << ":" << r.line_no << ": " << util::ansi_highlight(r.matched_line, opt.pattern) << "\n";
                } else {
                    cout << r.path << ":" << r.line_no << ": " << r.matched_line << "\n";
                }
            } else {
                if (r.is_dir && opt.color) {
                    cout << "\x1b[1;34m" << r.path << "\x1b[0m\n";
                } else {
                    cout << r.path << "\n";
                }
            }
        }
    });

    if (opt.json_array) cout << "\n]\n";

    return 0;
}


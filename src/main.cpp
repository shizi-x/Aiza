#include <iostream>
#include <string>
#include <vector>
#include "search.hpp"
#include "utils.hpp"

using namespace std;

int main(int argc, char** argv) {
    SearchOptions opt;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "-d" && i + 1 < argc) {
            opt.find_dirs = true;
            opt.pattern = argv[++i];
        }
        else if (a == "-f" && i + 1 < argc) {
            opt.find_files = true;
            opt.pattern = argv[++i];
        }
        else if (a == "-z" && i + 1 < argc) {
            opt.content_substr = true;
            opt.pattern = argv[++i];
        }
        else if (a == "-R" && i + 1 < argc) {
            opt.content_regex = true;
            opt.regex_pattern = argv[++i];
        }
        else if (a == "-rx" && i + 1 < argc) {
            opt.regex_pattern = argv[++i];
            opt.pattern = argv[i];
        }
        else if (a == "--json") opt.json = true;
        else if (a == "--json-array") opt.json_array = true;
        else if (a == "--no-color") opt.color = false;
        else if (a == "--fuzzy") opt.fuzzy = true;
        else if (a == "-i") opt.icase = true;
        else if (a == "-t" && i + 1 < argc) opt.threads = stoul(argv[++i]);
        else if (a == "--ignore-file" && i + 1 < argc) opt.ignore.load_from_file(argv[++i]);
        else opt.roots.push_back(a);
    }

    if (opt.roots.empty()) opt.roots.push_back(".");

    for (const auto& root : opt.roots) {
        opt.path = root;
        Searcher searcher(opt);
        searcher.run([&opt](const Result& r) {
            if (opt.json || opt.json_array) {
                cout << "{ \"path\": \"" << r.path.string() << "\", \"line_no\": " << r.line_no
                     << ", \"matched_line\": \"" << r.matched_line << "\" }";
                if (opt.json_array) cout << ",\n"; else cout << "\n";
            } else {
                if (opt.color) {
                    cout << r.path << ":" << r.line_no << ": "
                         << util::ansi_highlight(r.matched_line, opt.pattern) << "\n";
                } else {
                    cout << r.path << ":" << r.line_no << ": " << r.matched_line << "\n";
                }
            }
        });
    }

    if (opt.json_array) cout << "\n]\n";

    return 0;
}


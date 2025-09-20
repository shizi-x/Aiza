#pragma once

#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <iostream>

// Minimal Ignore stub (so Aiza compiles)
struct Ignore {
    void load_from_file(const std::string&) {
        // do nothing for now
    }
};

struct Result {
    std::filesystem::path path;
    std::string matched_line;
    size_t line_no;
};

// Callback for search results
using ResultCallback = std::function<void(const Result&)>;

// Search options
struct SearchOptions {
    // Search type
    bool find_dirs = false;
    bool find_files = false;
    bool content_substr = false;
    bool content_regex = false;

    // Regex / substring pattern
    std::string pattern;
    std::string regex_pattern;

    // File options
    bool json = false;
    bool json_array = false;
    bool color = true;
    bool fuzzy = false;
    bool icase = false;
    unsigned threads = 1;

    // Directories / roots
    std::vector<std::string> roots;

    // Ignore file (stub)
    Ignore ignore;

    // Starting path for search
    std::string path = ".";  
};

class Searcher {
public:
    explicit Searcher(const SearchOptions &options);
    void run(const ResultCallback &cb);

private:
    SearchOptions opt;
};


#pragma once
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <regex>
#include "ignore.hpp"

struct SearchOptions {
    bool find_dirs = false;
    bool find_files = false;
    bool content_substr = false;
    bool content_regex = false;
    bool binary_search = false; // search raw bytes
    bool fuzzy = false;
    bool icase = false;
    bool json = false;
    bool json_array = false;
    bool show_line_numbers = false;
    bool color = true;
    int fuzzy_threshold = 2;
    size_t threads = 4;
    std::string pattern;       // for names or substring
    std::string regex_pattern; // for PCRE2 regex (if set)
    std::vector<std::string> roots;
    IgnoreMatcher ignore;
};

struct Result {
    std::filesystem::path path;
    bool is_dir = false;
    std::string matched_line; // for content
    size_t line_no = 0;
};

using ResultCallback = std::function<void(const Result&)>;

class Searcher {
public:
    explicit Searcher(const SearchOptions &opt);
    void run(const ResultCallback &cb);
private:
    SearchOptions opt;
};


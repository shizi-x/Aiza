#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace util {
    std::string glob_to_regex(const std::string &glob);
    bool matches_glob(const std::string &name, const std::string &pattern, bool icase);
    bool looks_like_binary(const std::filesystem::path &p);
    int levenshtein(const std::string &a, const std::string &b);
    std::string ansi_highlight(const std::string &s, const std::string &match);
    // memmem-like search for bytes
    const void* memmem_portable(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len);
}


#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <optional>

struct IgnoreRule {
    std::string pattern;
    bool negation = false;      // '!' rule
    bool directory_only = false; // trailing '/'
    bool anchored = false;      // leading '/' (match from root)
};

class IgnoreMatcher {
public:
    void load_from_file(const std::filesystem::path &p);
    bool is_ignored(const std::filesystem::path &root, const std::filesystem::path &entry) const;
private:
    std::vector<IgnoreRule> rules;
    // helper
    bool match_rule(const IgnoreRule &r, const std::filesystem::path &root, const std::filesystem::path &entry) const;
};


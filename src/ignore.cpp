#include "ignore.hpp"
#include "utils.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

static string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a==string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

void IgnoreMatcher::load_from_file(const fs::path &p) {
    rules.clear();
    ifstream in(p);
    if (!in) return;
    string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        IgnoreRule r;
        if (line[0] == '!') { r.negation = true; line = line.substr(1); }
        if (!line.empty() && line.back() == '/') { r.directory_only = true; line.pop_back(); }
        if (!line.empty() && line.front() == '/') { r.anchored = true; line = line.substr(1); }
        r.pattern = line;
        rules.push_back(r);
    }
}

// root: directory where ignore file is considered relative to
bool IgnoreMatcher::is_ignored(const fs::path &root, const fs::path &entry) const {
    // We process all rules; last matching rule determines result (like gitignore)
    if (rules.empty()) return false;
    bool ignored = false;
    for (const auto &r : rules) {
        if (match_rule(r, root, entry)) {
            ignored = !r.negation;
        }
    }
    return ignored;
}

bool IgnoreMatcher::match_rule(const IgnoreRule &r, const fs::path &root, const fs::path &entry) const {
    // Build a relative path from root to entry (posix style)
    fs::path rel;
    try {
        rel = fs::relative(entry, root);
    } catch (...) {
        rel = entry;
    }
    string rels = rel.generic_string(); // use forward slashes
    string name = entry.filename().string();

    // If rule is directory_only, ensure entry is a directory
    bool is_dir = fs::is_directory(entry);
    if (r.directory_only && !is_dir) return false;

    // If anchored: match from root of the ignore file -> so compare rels to pattern with glob
    if (r.anchored) {
        return util::matches_glob(rels, r.pattern, false);
    }

    // Not anchored: rule can match anywhere in the path segments
    // We'll check:
    //  - full relative path
    //  - basename only
    if (util::matches_glob(rels, r.pattern, false)) return true;
    if (util::matches_glob(name, r.pattern, false)) return true;

    // Also check each path component
    for (auto it = rel.begin(); it != rel.end(); ++it) {
        string comp = it->string();
        if (util::matches_glob(comp, r.pattern, false)) return true;
    }

    return false;
}


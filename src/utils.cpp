#include "utils.hpp"
#include <regex>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <cstring>
#include <sys/stat.h>

using namespace std;
namespace fs = std::filesystem;

namespace util {

string glob_to_regex(const string &glob) {
    string out; out.reserve(glob.size()*2);
    for (size_t i = 0; i < glob.size(); ++i) {
        char c = glob[i];
        switch (c) {
            case '*':
                // support ** by translating to .*
                out += ".*";
                break;
            case '?': out += '.'; break;
            case '.': out += "\\."; break;
            case '\\': out += "\\\\"; break;
            default:
                if (string("^$+()[]{}|").find(c) != string::npos) out += '\\';
                out += c;
        }
    }
    return "^" + out + "$";
}

bool matches_glob(const string &name, const string &pattern, bool icase) {
    try {
        string rx = glob_to_regex(pattern);
        std::regex::flag_type flags = std::regex::ECMAScript;
        if (icase) flags |= std::regex::icase;
        std::regex r(rx, flags);
        return std::regex_match(name, r);
    } catch (...) {
        return false;
    }
}

bool looks_like_binary(const fs::path &p) {
    constexpr size_t CHECK = 512;
    std::ifstream in(p, std::ios::binary);
    if (!in) return true;
    std::vector<char> buf(CHECK);
    in.read(buf.data(), buf.size());
    std::streamsize n = in.gcount();
    for (std::streamsize i = 0; i < n; ++i) if (buf[i] == '\0') return true;
    return false;
}

int levenshtein(const string &a, const string &b) {
    if (a.empty()) return (int)b.size();
    if (b.empty()) return (int)a.size();
    vector<int> prev(b.size()+1), cur(b.size()+1);
    for (size_t j=0;j<=b.size();++j) prev[j] = (int)j;
    for (size_t i=0;i<a.size();++i){
        cur[0] = (int)i+1;
        for (size_t j=0;j<b.size();++j){
            int cost = (a[i]==b[j])?0:1;
            cur[j+1] = min({ prev[j+1]+1, cur[j]+1, prev[j]+cost });
        }
        prev.swap(cur);
    }
    return prev[b.size()];
}

string ansi_highlight(const string &s, const string &match) {
    if (match.empty()) return s;
    string s_low = s, m_low = match;
    // case-sensitive find - keep original casing
    size_t pos = s.find(match);
    if (pos == string::npos) {
        // try case-insensitive
        transform(s_low.begin(), s_low.end(), s_low.begin(), ::tolower);
        transform(m_low.begin(), m_low.end(), m_low.begin(), ::tolower);
        pos = s_low.find(m_low);
        if (pos == string::npos) return s;
    }
    const string start = "\x1b[1;31m";
    const string end = "\x1b[0m";
    return s.substr(0,pos) + start + s.substr(pos, match.size()) + end + s.substr(pos + match.size());
}

// portable memmem fallback
const void* memmem_portable(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len) {
    if (!needle_len) return haystack;
    if (!haystack || haystack_len < needle_len) return nullptr;
    const unsigned char *h = (const unsigned char*)haystack;
    const unsigned char *n = (const unsigned char*)needle;
    for (size_t i=0; i + needle_len <= haystack_len; ++i) {
        if (h[i] == n[0] && memcmp(h + i, n, needle_len) == 0) return h + i;
    }
    return nullptr;
}

} // namespace util


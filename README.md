<h1>AIZA</h1>

Unified fast search tool for Linux written in C++17.

Features:
- Find files and directories by glob or fuzzy name
- Recursive content search (substring & PCRE2 regex)
- Name+content combined mode (`-rx`)
- Parallel file scanning with a small thread pool
- `.aizaignore` support with gitignore-like rules (negation, globs, directory patterns)
- Colorized ANSI output and proper JSON output (line-delimited JSON objects or JSON array via `--json-array`)
- Binary / raw-byte search mode
- mmap-based scanning for high performance on large files
- Progress indicator

<h2>Build</h2>

Requirements:
- C++17 compiler (GCC/Clang)
- CMake >= 3.14
- libpcre2-8-dev (PCRE2)
- Internet access to fetch nlohmann/json (or vendor the header locally)

Example:
```bash
sudo apt install build-essential cmake libpcre2-dev   # Debian/Ubuntu example
mkdir build && cd build
cmake ..
cmake --build . -j

Options (high-level):
-d <pattern> : find directories matching glob
-f <pattern> : find files matching glob
-z <text> : substring content search
-R <regex> : PCRE2 regex content search
-rx <pattern>: heuristics: match both name (glob/ fuzzy) and contents (regex)
-i : case-insensitive (applies to substring and name matches)
--binary : treat content search as raw byte pattern (no text filtering)
--ignore-file <file> : load .aizaignore (gitignore-like) from a path
--json : emit line-delimited JSON objects
--json-array : emit a single JSON array (useful if you want a single JSON file)
--no-color : disable ANSI color output
-t <n> : number of threads
--fuzzy : enable fuzzy (Levenshtein) name matching

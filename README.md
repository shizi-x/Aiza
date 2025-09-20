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
- CMake >= 3.5
- libpcre2-8-dev (PCRE2)
- Internet access to fetch nlohmann/json (or vendor the header locally)

<b>Debian/Ubuntu-based</b>
```bash
sudo apt install -y build-essential cmake libpcre2-dev 
mkdir build && cd build
cmake ..
cmake --build . -j
```
<b>RHEL-based</b>
```bash
sudo dnf install -y build-essential cmake libpcre2-dev 
mkdir build && cd build
cmake ..
cmake --build . -j
```
<b>Arch-based</b>
```bash
sudo pacman -Syu --no-confirm build-essential cmake libpcre2-dev    
mkdir build && cd build
cmake ..
cmake --build . -j
```

<p>Options (high-level):</p>
-d <pattern> : find directories matching glob<br>
-f <pattern> : find files matching glob<br>
-z <text> : substring content search<br>
-R <regex> : PCRE2 regex content search<br>
-rx <pattern>: heuristics: match both name (glob/ fuzzy) and contents (regex)<br>
-i : case-insensitive (applies to substring and name matches)<br>
--binary : treat content search as raw byte pattern (no text filtering)<br>
--ignore-file <file> : load .aizaignore (gitignore-like) from a path<br>
--json : emit line-delimited JSON objects<br>
--json-array : emit a single JSON array (useful if you want a single JSON file)<br>
--no-color : disable ANSI color output<br>
-t <n> : number of threads<br>
--fuzzy : enable fuzzy (Levenshtein) name matching<br>
</p>

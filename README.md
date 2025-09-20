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
cmake --build . -j$(nproc)
```
<b>RHEL-based</b>
```bash
sudo dnf install -y build-essential cmake libpcre2-dev 
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```
<b>Arch-based</b>
```bash
sudo pacman -Syu --no-confirm build-essential cmake libpcre2-dev    
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

<h3>Options (high-level):</h3><br>
<b>-d <pattern> :</b> <i>find directories matching glob</i><br>
<b>-f <pattern> :</b> <i>find files matching glob</i><br>
<b>-z <text> :</b> <i>substring content search</i><br>
<b>-R <regex> :</b> <i>PCRE2 regex content search</i><br>
<b>-rx <pattern>:</b> <i>heuristics: match both name (glob/ fuzzy) and contents (regex)</i><br>
<b>-i :</b> <i>case-insensitive (applies to substring and name matches)</i><br>
<b>--binary :</b> <i>treat content search as raw byte pattern (no text filtering)</i><br>
<b>--ignore-file</b> <file> : <i>load .aizaignore (gitignore-like) from a path</i><br>
<b>--json :</b> <i>emit line-delimited JSON objects</i><br>
<b>--json-array :</b> <i>emit a single JSON array (useful if you want a single JSON file)</i><br>
<b>--no-color :</b> <i>disable ANSI color output</i><br>
<b>-t <n> :</b> <i>number of threads</i><br>
<b>--fuzzy :</b> <i>enable fuzzy (Levenshtein) name matching</i><br>
</p>

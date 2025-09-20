#!/usr/bin/env bash
set -e

echo "-----------------------------"
echo "Aiza Installer"
echo "-----------------------------"

# 1. Detect OS and install dependencies
echo "Checking dependencies..."
if command -v apt &>/dev/null; then
    PKG_MANAGER="apt"
    INSTALL_CMD="sudo apt install -y build-essential cmake libpcre2-dev git"
elif command -v dnf &>/dev/null; then
    PKG_MANAGER="dnf"
    INSTALL_CMD="sudo dnf install -y gcc-c++ cmake pcre2-devel git"
elif command -v pacman &>/dev/null; then
    PKG_MANAGER="pacman"
    INSTALL_CMD="sudo pacman -S --needed --noconfirm base-devel cmake pcre2 git"
else
    echo "Unsupported Linux distro. Please install dependencies manually."
    exit 1
fi

echo "Installing dependencies via $PKG_MANAGER..."
$INSTALL_CMD

# 2. Create build directory
mkdir -p build
cd build

# 3. Configure with CMake
echo "Configuring project..."
cmake ..

# 4. Build
echo "Building Aiza..."
cmake --build . -j$(nproc)

# 5. Install binary
INSTALL_DIR="/usr/local/bin"
if [ ! -w "$INSTALL_DIR" ]; then
    echo "Sudo permissions required to install in $INSTALL_DIR"
    sudo cp aiza "$INSTALL_DIR"
else
    cp aiza "$INSTALL_DIR"
fi

echo "Installation complete! You can now run 'aiza' from anywhere."


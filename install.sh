#!/usr/bin/env bash
set -e

echo "--------------------------"
echo "  Aiza v0.7.9 Installer"
echo "--------------------------"

# -------------------------------
# Step 1: Detect dependencies
# -------------------------------
echo "Checking dependencies..."
DEPENDENCIES=(git cmake g++)

for dep in "${DEPENDENCIES[@]}"; do
    if ! command -v "$dep" &> /dev/null; then
        MISSING_DEPENDENCIES+=("$dep")
    fi
done

if [ "${#MISSING_DEPENDENCIES[@]}" -gt 0 ]; then
    echo "Missing dependencies: ${MISSING_DEPENDENCIES[*]}"
    echo "Installing dependencies..."
    if command -v apt &>/dev/null; then
        sudo apt update
        sudo apt install -y build-essential cmake libpcre2-dev git
    elif command -v dnf &>/dev/null; then
        sudo dnf install -y gcc-c++ cmake pcre2-devel git
    elif command -v pacman &>/dev/null; then
        sudo pacman -Syu --noconfirm base-devel cmake pcre2 git
    else
        echo "Unsupported Linux distro. Please install dependencies manually."
        exit 1
    fi
fi

# -------------------------------
# Step 2: Build Aiza
# -------------------------------
echo "Creating build directory..."
mkdir -p build
cd build

echo "Configuring project..."
cmake ..

echo "Building project..."
cmake --build . -j$(nproc)

# -------------------------------
# Step 3: Install binary
# -------------------------------
INSTALL_DIR="/usr/local/bin"
if [ ! -w "$INSTALL_DIR" ]; then
    echo "No write permission for $INSTALL_DIR. Asking for sudo..."
    sudo cp aiza "$INSTALL_DIR" || {
        echo "Sudo failed or canceled. Installing locally in \$HOME/.local/bin..."
        INSTALL_DIR="$HOME/.local/bin"
        mkdir -p "$INSTALL_DIR"
        cp aiza "$INSTALL_DIR"
    }
else
    cp aiza "$INSTALL_DIR"
fi

# -------------------------------
# Step 4: Add to PATH if needed
# -------------------------------
if ! echo "$PATH" | grep -q "$INSTALL_DIR"; then
    echo ""
    echo "Add the following line to your shell config (~/.bashrc, ~/.zshrc):"
    echo "export PATH=\"\$PATH:$INSTALL_DIR\""
fi

echo "---------------------------------------------------"
echo "  Installation complete! You can now run 'aiza'."
echo "---------------------------------------------------"


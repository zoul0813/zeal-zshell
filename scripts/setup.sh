#!/bin/bash

# Setup script for ZShell menuconfig system

set -e

KCONFIG_VERSION="4.6.0.0"
KCONFIG_URL="http://ymorin.is-a-geek.org/download/kconfig-frontends/kconfig-frontends-${KCONFIG_VERSION}.tar.xz"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "Setting up ZShell menuconfig system..."

# Create necessary directories
mkdir -p "$PROJECT_DIR/scripts/kconfig"
mkdir -p "$PROJECT_DIR/include"

# Check if kconfig tools are already installed system-wide
if command -v kconfig-mconf >/dev/null 2>&1; then
    echo "Found system-wide kconfig tools, creating symlinks..."
    ln -sf "$(which kconfig-mconf)" "$PROJECT_DIR/scripts/kconfig/mconf"
    ln -sf "$(which kconfig-conf)" "$PROJECT_DIR/scripts/kconfig/conf"
elif command -v mconf >/dev/null 2>&1; then
    echo "Found system-wide mconf, creating symlinks..."
    ln -sf "$(which mconf)" "$PROJECT_DIR/scripts/kconfig/mconf"
    ln -sf "$(which conf)" "$PROJECT_DIR/scripts/kconfig/conf"
else
    echo "Kconfig tools not found. Please install them:"
    echo ""
    echo "On Ubuntu/Debian:"
    echo "  sudo apt-get install kconfig-frontends"
    echo ""
    echo "On macOS with Homebrew:"
    echo "  brew install kconfig-frontends"
    echo ""
    echo "Or build from source:"
    echo "  wget $KCONFIG_URL"
    echo "  tar xf kconfig-frontends-${KCONFIG_VERSION}.tar.xz"
    echo "  cd kconfig-frontends-${KCONFIG_VERSION}"
    echo "  ./configure --enable-mconf --enable-nconf --enable-conf"
    echo "  make"
    echo "  sudo make install"
    echo ""
    exit 1
fi

# Make the generate_config.py script executable
chmod +x "$PROJECT_DIR/scripts/generate_config.py"

# Create build directory and initial configuration
echo "Setting up CMake build directory..."
mkdir -p "$PROJECT_DIR/build"
cd "$PROJECT_DIR"

# Create initial .config if it doesn't exist
if [ ! -f "$PROJECT_DIR/.config" ]; then
    echo "Creating default configuration..."
    cmake -B build
    cmake --build build --target defconfig
fi

echo "Setup complete!"
echo ""
echo "Usage:"
echo "  cmake -B build                           - Configure CMake"
echo "  cmake --build build --target menuconfig - Interactive configuration"
echo "  cmake --build build --target config     - Text-based configuration"
echo "  cmake --build build --target defconfig  - Reset to defaults"
echo "  cmake --build build                     - Build the shell"
echo "  cmake --build build --target help       - Show all targets"
echo ""
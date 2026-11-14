#!/usr/bin/env bash
set -euo pipefail

# Copy a matching runtime and its libraries from an OPFS emulator build.
SOURCE_PATH="${1:?Usage: setup.sh /path/to/Zeal-NativeEmulator/build}"
DEST_PATH="$(cd "$(dirname "$0")" && pwd)"

cp "$SOURCE_PATH/zeal.elf.js" "$SOURCE_PATH/zeal.elf.wasm" "$DEST_PATH/"
mkdir -p "$DEST_PATH/libnative"
cp "$SOURCE_PATH/libnative/"*.js "$DEST_PATH/libnative/"

cp "$DEST_PATH/../docs/images/logo.svg" "$DEST_PATH/logo.svg"

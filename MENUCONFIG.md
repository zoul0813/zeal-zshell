# ZShell Menuconfig System

This project now includes a menuconfig system for easy configuration management, similar to what's used in the Linux kernel.

## Setup

1. Run the setup script to install dependencies:
   ```bash
   ./scripts/setup.sh
   ```

2. If you're on Ubuntu/Debian, you can install kconfig-frontends directly:
   ```bash
   sudo apt-get install kconfig-frontends
   ```

## Usage

### Setup CMake
```bash
cmake -B build
```
This configures the CMake build system.

### Interactive Configuration
```bash
cmake --build build --target menuconfig
```
This opens a graphical menu where you can configure all options interactively.

### Text-based Configuration  
```bash
cmake --build build --target config
```
This provides a text-based question/answer configuration interface.

### Reset to Defaults
```bash
cmake --build build --target defconfig
```
This resets all configuration options to their default values.

### Update Configuration
```bash
cmake --build build --target oldconfig
```
This updates your existing configuration with any new options that have been added.

### Build
```bash
cmake --build build
```
This builds the shell with your current configuration.

## Configuration Options

### Buffer and Memory Settings
- **COMMAND_MAX**: Maximum command buffer size (64-1024 bytes, default: 256)
- **MAX_PATHS**: Maximum number of paths (1-32, default: 8)

### History Settings
- **HISTORY_ENABLED**: Enable/disable command history (default: enabled)
- **HISTORY_MAX_ENTRIES**: Maximum history entries (1-64, default: 10)

### Features
- **AUTOEXEC_ENABLED**: Enable/disable autoexec processing (default: enabled)
- **COLOR_SUPPORT**: Enable/disable ANSI color support (default: enabled)
- **PROMPT_CUSTOMIZATION**: Enable/disable prompt customization (default: enabled)
- **TAB_COMPLETION**: Enable/disable tab completion (default: disabled)
- **BUILTIN_COMMANDS**: Enable/disable built-in commands (default: enabled)

### Debug and Development
- **DEBUG_MODE**: Enable/disable debug mode (default: disabled)
- **VERBOSE_ERRORS**: Enable/disable verbose error messages (default: enabled)

## Files

- `Kconfig`: Configuration definition file
- `CMakeLists.txt`: CMake build configuration with menuconfig integration
- `.config`: Generated configuration file (do not edit manually)
- `include/config.h`: Generated C header with configuration defines
- `scripts/generate_config.py`: Script to convert .config to config.h
- `scripts/setup.sh`: Setup script for dependencies

## How It Works

1. The `Kconfig` file defines all available configuration options
2. Running `cmake --build build --target menuconfig` allows you to set these options interactively  
3. Your choices are saved in `.config`
4. The `generate_config.py` script converts `.config` to `include/config.h`
5. CMake automatically regenerates `config.h` when `.config` changes
6. Your C code includes `config.h` and uses the `CONFIG_*` defines

## Migrating Existing Code

The system maintains backward compatibility by providing the old constant names:
- `COMMAND_MAX` maps to `CONFIG_COMMAND_MAX`
- `MAX_PATHS` maps to `CONFIG_MAX_PATHS`
- `HISTORY_ENABLED` maps to `CONFIG_HISTORY_ENABLED`
- `AUTOEXEC_ENABLED` maps to `CONFIG_AUTOEXEC_ENABLED`

## Adding New Options

To add a new configuration option:

1. Edit `Kconfig` and add your option
2. Run `cmake --build build --target menuconfig` to configure it
3. Use `#ifdef CONFIG_YOUR_OPTION` in your C code
4. Optionally add backward compatibility defines in `generate_config.py`
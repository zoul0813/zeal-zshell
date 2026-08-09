# ZShell Menuconfig System

This project now includes a menuconfig system for easy configuration management, similar to what's used in the Linux kernel.

## Usage

### Interactive Configuration

```bash
zde cmake --target menuconfig
```

This opens a graphical menu where you can configure all options interactively.

### Reset to Defaults

```bash
zde cmake --target defconfig
```

This resets all configuration options to their default values.

### Update Configuration

```bash
zde cmake --target oldconfig
```

This updates your existing configuration with any new options that have been added.

### Build

```bash
zde cmake
```

This builds the shell with your current configuration.

## Configuration Options

### Buffer and Memory Settings

- **COMMAND_MAX**: Maximum command buffer size (64-256 bytes, default: 256)
- **MAX_PATHS**: Maximum number of paths (1-16, default: 8)
- **BATCH_MAX_DEPTH**: Maximum active nested batch scripts (1-16, default: 4)

Combined command, history, PATH, and nested-batch storage is limited to 15,000 bytes. Unsafe manual `.config` combinations fail during header generation.

### History Settings

- **HISTORY_ENABLED**: Enable/disable command history (default: enabled)
- **HISTORY_MAX_ENTRIES**: Maximum history entries (1-32, default: 10)

### Features

- **AUTOEXEC_ENABLED**: Enable/disable autoexec processing (default: enabled)
- **COLOR_SUPPORT**: Enable/disable ANSI color support (default: enabled)

### Built-in Commands

The `#`, `pwd`, `exit`, `exec`, `clear`, `which`, `true`, `false`, `ver`, and `reset` commands each have an enabled-by-default option. Disabled handlers are omitted from both the command list and the linked binary.

The internal `cd`, `help`, and `set` commands are always built. `history` is controlled only by **HISTORY_ENABLED**.

## Files

- `Kconfig`: Configuration definition file
- `CMakeLists.txt`: CMake build configuration with menuconfig integration
- `.config`: Generated configuration file (do not edit manually)
- `src/config.h`: Generated C header with configuration defines
- `scripts/generate_config.py`: Script to convert .config to config.h

## How It Works

1. The `Kconfig` file defines all available configuration options
2. Running `zde cmake --target menuconfig` allows you to set these options interactively
3. Your choices are saved in `.config`
4. The `generate_config.py` script converts `.config` to `src/config.h`
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
2. Run `zde cmake --target menuconfig` to configure it
3. Use `#ifdef CONFIG_YOUR_OPTION` in your C code
4. Optionally add backward compatibility defines in `generate_config.py`

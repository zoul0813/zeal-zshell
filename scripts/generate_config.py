#!/usr/bin/env python3
"""
Generate config.h from Kconfig .config file
"""

import sys
import os
import re

CONFIG_LIMITS = {
    "COMMAND_MAX": (64, 256, 256),
    "MAX_PATHS": (1, 16, 8),
    "BATCH_MAX_DEPTH": (1, 16, 4),
    "HISTORY_MAX_ENTRIES": (1, 32, 10),
}
CONFIGURABLE_STORAGE_BUDGET = 15000
ZEAL_PATH_MAX = 128
BUILTIN_DEFAULTS = (
    "HASH",
    "PWD",
    "EXIT",
    "EXEC",
    "CLEAR",
    "WHICH",
    "TRUE",
    "FALSE",
    "VER",
    "RESET",
)
SUPPORTED_CONFIGS = set(CONFIG_LIMITS) | {
    "AUTOEXEC_ENABLED",
    "AUTOEXEC_FILENAME",
    "COLOR_SUPPORT",
    "HISTORY_ENABLED",
} | {f"BUILTIN_{name}" for name in BUILTIN_DEFAULTS}


def parse_config(config_file):
    """Parse the .config file and return a dictionary of config values."""
    config = {}

    if not os.path.exists(config_file):
        print(f"Warning: Config file {config_file} does not exist")
        return config

    with open(config_file, "r") as f:
        for line in f:
            line = line.strip()

            match = re.match(r"# CONFIG_([A-Z_]+) is not set$", line)
            if match:
                config[match.group(1)] = False
                continue

            # Skip comments and empty lines
            if line.startswith("#") or not line:
                continue

            # Parse CONFIG_NAME=value
            match = re.match(r"CONFIG_([A-Z_]+)=(.+)", line)
            if match:
                name = match.group(1)
                value = match.group(2)

                # Handle different value types
                if value == "y":
                    config[name] = True
                elif value == "n":
                    config[name] = False
                elif value.startswith('"') and value.endswith('"'):
                    # String value
                    config[name] = value[1:-1]  # Remove quotes
                else:
                    # Try to parse as integer, otherwise keep as string
                    try:
                        config[name] = int(value)
                    except ValueError:
                        config[name] = value

    return config


def validate_config(config):
    """Reject unsafe numeric values and aggregate static-memory usage."""
    values = {}
    for key, (minimum, maximum, default) in CONFIG_LIMITS.items():
        value = config.get(key, default)
        if type(value) is not int or not minimum <= value <= maximum:
            raise ValueError(
                f"CONFIG_{key} must be an integer from {minimum} to {maximum}"
            )
        values[key] = value

    command_max = values["COMMAND_MAX"]
    history_storage = 0
    if config.get("HISTORY_ENABLED") is True:
        history_storage = values["HISTORY_MAX_ENTRIES"] * (command_max + 5)
    configured_storage = (
        command_max
        + history_storage
        + values["MAX_PATHS"] * ZEAL_PATH_MAX
        + values["BATCH_MAX_DEPTH"] * command_max
    )
    if configured_storage > CONFIGURABLE_STORAGE_BUDGET:
        raise ValueError(
            "configured static storage "
            f"({configured_storage} bytes) exceeds "
            f"{CONFIGURABLE_STORAGE_BUDGET}-byte budget"
        )


def generate_header(config, output_file):
    """Generate the config.h header file."""

    with open(output_file, "w") as f:
        f.write("/* Automatically generated config.h - DO NOT EDIT */\n")
        f.write("#ifndef CONFIG_H\n")
        f.write("#define CONFIG_H\n\n")

        # Sort keys for consistent output
        for key in sorted(config.keys()):
            if key not in SUPPORTED_CONFIGS:
                continue
            value = config[key]

            if isinstance(value, bool):
                if value:
                    f.write(f"#define CONFIG_{key} 1\n")
                else:
                    f.write(f"/* #undef CONFIG_{key} */\n")
            elif isinstance(value, int):
                f.write(f"#define CONFIG_{key} {value}\n")
            elif isinstance(value, str):
                f.write(f'#define CONFIG_{key} "{value}"\n')

        f.write("\n")

        # Add convenience macros
        f.write("/* Convenience macros */\n")
        f.write("#ifdef CONFIG_HISTORY_ENABLED\n")
        f.write("#define HISTORY_ENABLED 1\n")
        f.write("#else\n")
        f.write("#define HISTORY_ENABLED 0\n")
        f.write("#endif\n\n")

        f.write("#ifdef CONFIG_AUTOEXEC_ENABLED\n")
        f.write("#define AUTOEXEC_ENABLED 1\n")
        f.write("#else\n")
        f.write("#define AUTOEXEC_ENABLED 0\n")
        f.write("#endif\n\n")

        f.write("#ifndef CONFIG_AUTOEXEC_FILENAME\n")
        f.write("#define CONFIG_AUTOEXEC_FILENAME \"b:/autoexec.zs\"\n")
        f.write("#endif\n")
        f.write("#define AUTOEXEC_FILENAME CONFIG_AUTOEXEC_FILENAME\n\n")

        for name in BUILTIN_DEFAULTS:
            key = f"BUILTIN_{name}"
            if key not in config:
                f.write(f"#define CONFIG_{key} 1\n")
        f.write("\n")

        # Add default values for backward compatibility
        f.write("/* Default values for constants */\n")
        f.write("#ifndef CONFIG_COMMAND_MAX\n")
        f.write("#define CONFIG_COMMAND_MAX 256\n")
        f.write("#endif\n")
        f.write("#define COMMAND_MAX CONFIG_COMMAND_MAX\n\n")

        f.write("#ifndef CONFIG_MAX_PATHS\n")
        f.write("#define CONFIG_MAX_PATHS 8\n")
        f.write("#endif\n")
        f.write("#define MAX_PATHS CONFIG_MAX_PATHS\n\n")

        f.write("#ifndef CONFIG_BATCH_MAX_DEPTH\n")
        f.write("#define CONFIG_BATCH_MAX_DEPTH 4\n")
        f.write("#endif\n")
        f.write("#define BATCH_MAX_DEPTH CONFIG_BATCH_MAX_DEPTH\n\n")

        f.write("#ifndef CONFIG_HISTORY_MAX_ENTRIES\n")
        f.write("#define CONFIG_HISTORY_MAX_ENTRIES 10\n")
        f.write("#endif\n")
        f.write("#define HISTORY_MAX CONFIG_HISTORY_MAX_ENTRIES\n")

        f.write("#endif /* CONFIG_H */\n")


def main():
    if len(sys.argv) != 3:
        print("Usage: generate_config.py <config_file> <output_file>")
        sys.exit(1)

    config_file = sys.argv[1]
    output_file = sys.argv[2]

    # Create output directory if it doesn't exist
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    config = parse_config(config_file)
    try:
        validate_config(config)
    except ValueError as error:
        print(f"Error: {error}", file=sys.stderr)
        sys.exit(1)
    generate_header(config, output_file)

    print(f"Generated {output_file} from {config_file}")


if __name__ == "__main__":
    main()

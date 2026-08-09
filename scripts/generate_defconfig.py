#!/usr/bin/env python3
"""Generate a complete .config using defaults declared in Kconfig."""

import sys

import kconfiglib


def main():
    if len(sys.argv) not in (3, 4) or (
        len(sys.argv) == 4 and sys.argv[3] != "--oldconfig"
    ):
        print(
            "Usage: generate_defconfig.py <Kconfig> <output> [--oldconfig]",
            file=sys.stderr,
        )
        return 1

    kconfig = kconfiglib.Kconfig(sys.argv[1])
    if len(sys.argv) == 4:
        kconfig.load_config(sys.argv[2])
    kconfig.write_config(sys.argv[2])
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Generate the demo welcome file as CP437 bytes with 16-color ANSI SGR.

Edit the Unicode artwork/text below, then run this script from any directory.
Display the generated file with coreutils acat, not plain cat.
"""

from pathlib import Path


LOGO = (
    '▄███████████████████████▄     ',
    '██▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀██     ',
    '▀██▄  ████████████▀   ▄██     ',
    '  ▀█▄  ▀▀▀▀▀▀▀███   ███▀      ',
    '   ▀██▄    ▄▄██▀ ▄███▀        ',
    '     ▀    ▄██▀ ▄████▄▄▄▄▄▄    ',
    '        ▄██  ▄██████████████▄ ',
    '     ▄▄██▀ ▄██▀            ▀█▄',
    '   ▄▄██▀ ▄███▀              ██',
    ' ▄███   ██████████  >_      ██',
    ' ██▀                      ▄██▀',
    ' ▀█████████████████████████▀  ',
)

# Each entry is (ANSI foreground color, text); all colors are supported by acat.
WELCOME = (
    (95, "ZShell + coreutils"),
    (37, "Zeal 8-bit OS / Interactive demo"),
    (37, ""),
    (97, "Welcome to your Zeal."),
    (37, "Apps and games live on T:/"),
    (37, ""),
    (95, "TRY A COMMAND"),
    (96, "tree     Explore your files"),
    (96, "zc       Open Zeal Commander"),
    (96, "ls T:/games/"),
    (37, "More launch tips: about-games"),
    (37, ""),
    (37, "help     List built-in commands"),
    (37, "Up/Down  Browse command history"),
)


LESSONS = (
    ("about-shell", "Start here: commands and built-ins"),
    ("about-keys", "Edit commands and browse history"),
    ("about-paths", "Find programs and scripts with PATH"),
    ("about-files", "Navigate drives, directories, and files"),
    ("about-zscript", "Read and run a real ZScript"),
    ("about-status", "Use success/failure conditionals"),
    ("about-disks", "Understand browser storage and resets"),
    ("about-fonts", "Explore fonts and the guest display"),
    ("about-games", "Find and launch games on T:/"),
    ("about-demos", "Discover demos and applications"),
)


def color(code):
    # Explicitly clear bold/dim; acat's SGR 0 alone does not reset that state.
    return f"\x1b[22;{code};40m"


def generate():
    border = color(35)
    lines = [border + "╔" + "═" * 78 + "╗"]
    artwork = [" " * 30, *LOGO, " " * 30]
    for logo, (foreground, text) in zip(artwork, WELCOME, strict=True):
        if len(logo) != 30 or len(text) > 39:
            raise ValueError("Welcome banner exceeds its column layout")
        lines.append(
            border + "║  " + color(95) + logo + border + "  │  "
            + color(foreground) + text.ljust(39) + border + "  ║"
        )
    lines.append(border + "╚" + "═" * 78 + "╝")
    lines.extend(["", color(95) + "LEARN ZSHELL - type a lesson name and press Enter", ""])
    for name, description in LESSONS:
        lines.append(color(96) + "  " + name.ljust(15) + color(37) + description)
    lines.extend(["", color(37) + "Each lesson clears the screen. Type welcome to return to this menu."])
    # Leave room for acat's final newline and the shell prompt on an 80x40 screen.
    if len(lines) > 36:
        raise ValueError("Welcome menu exceeds one screen")
    # CRLF keeps both the guest acat renderer and desktop ANSI viewers aligned.
    # Finish in ordinary light gray on black so subsequent output is unaffected.
    return ("\r\n".join(lines) + "\r\n\x1b[0;22;37;40m").encode("cp437")


def main():
    destination = Path(__file__).resolve().parents[1] / ".deploy/eeprom/welcome.ans"
    destination.write_bytes(generate())
    print(f"Generated {destination.relative_to(destination.parents[2])} (80 columns; banner and lesson menu fit one screen)")


if __name__ == "__main__":
    main()

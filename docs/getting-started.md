# Getting started

[Documentation index](README.md)

## Install or try ZShell

Try the [browser demo](https://zoul0813.github.io/zeal-zshell/) for a system with
ZShell and coreutils already installed. Click the emulator screen to give it
keyboard focus. See [browser storage and controls](deployment.md) before changing
or clearing saved disk images.

For a Zeal 8-bit OS system, install ZShell and coreutils from the latest
[Zeal Software Collection community image](https://github.com/Zeal8bit/Zeal-Software-Collection/releases/latest):

1. Download the community TF image archive appropriate for your storage device
   and extract it.
2. Write the extracted image to your TF card using your usual image-writing tool,
   or attach it as the TF image in your emulator. Writing an image replaces the
   destination's contents; preserve files you need first.
3. Boot Zeal 8-bit OS with that image available as `T:`.
4. Run the bundled flasher with the ZShell ROM image:

   ```text
   t:/sys/flasher t:/sys/zshell.rom
   ```

5. After flashing completes successfully, reset the system to start ZShell with
   coreutils installed on the ROM disk.

The flasher installs the bundled ROM image; simply running ZShell from the TF
card does not install it to the ROM disk. ZShell's own `clear` command and the
browser's Clear Data control are unrelated to this installation procedure.

For custom builds, see [building from source](configuration.md). Standalone
[ZShell release binaries](https://github.com/zoul0813/zeal-zshell/releases) are
also available, but installing only that executable does not supply coreutils or
change the kernel's initial-shell setting. The supplied `os.conf` and `emu.conf`
select `a:/zshell` as the initial executable when building a custom OS image.

ZShell uses Zeal video and keyboard facilities and the Zeal VideoBoard SDK. It is
not a general-purpose host terminal shell. Its filesystem access and program
execution depend on the mounted drives and capabilities of the running OS.

## Drives and paths

Paths use a drive letter, colon, and forward slashes, such as `A:/zshell` or
`T:/bin/echo`. In the bundled demo, `A:` is the ROM disk, `B:` is the EEPROM disk,
and `T:` is the community TF image. Other OS configurations can mount different
letters or omit these disks.

```text
cd T:/
pwd
cd T:/bin/
cd ..
```

`cd` requires a path. Directory traversal and path validity are handled by the
OS. Use `pwd` to see the complete current path. An executable in the current
directory can be launched explicitly as `./program`; see
[command lookup](interactive-shell.md#command-lookup).

## First commands

```text
help
ver
set
set PATH=T:/apps/,T:/bin/,A:/,B:/
which echo
echo Hello from ZShell
history
```

A fresh shell starts with `PATH=A:/`. Autoexec can replace this before the first
prompt. PATH changes and command history are local to the running shell.

## Startup modes

With no arguments, `zshell` starts interactively: it initializes PATH, attempts
`ver`, processes autoexec if enabled, initializes history, and opens the prompt.
If `ver` has been compiled out, the startup version command cannot print a
version banner.

With a script path, it runs that script and exits with the script's status:

```text
zshell T:/scripts/demo.zs
zshell -q T:/scripts/demo.zs
zshell -h
```

| Argument | Behavior |
| --- | --- |
| No arguments | Start the interactive shell |
| `path` | Run a script with command echo enabled, then exit |
| `-q path` | Run a script without the shell's command echo, then exit |
| `-h` | Print usage and exit successfully |

The optional flags must precede the path in a single `-` group. Only `q` and `h`
are recognized; there is no long-option or `--` parser. A missing script path
(other than `-h`) or unknown option returns `ERR_INVALID_PARAMETER`. An oversized
path returns `ERR_PATH_TOO_LONG`.

The startup script path is opened directly rather than searched through PATH.
Script mode skips the interactive version banner, autoexec, and history setup.
The script receives no positional arguments: the remaining command-line text is
treated as its path. A startup script need not have a `.zs` suffix, though that
suffix is required for automatic script dispatch from the prompt.

## Autoexec

With `AUTOEXEC_ENABLED=y`, interactive startup checks `AUTOEXEC_FILENAME`, whose
default is `B:/autoexec.zs`. It runs the file quietly if it exists and is a regular
file. If it is missing or not a file, startup reports `Could not load ...` and
continues to the prompt. A script failure does not prevent interactive startup.

For example:

```text
; B:/autoexec.zs
set PATH=T:/apps/,T:/bin/,A:/,B:/
cd T:/
acat B:/welcome.ans
```

Autoexec uses the same [ZScript syntax](zscripts.md) as other scripts. It can set
PATH, change directory, or launch installed programs such as `setfont`.
These changes remain active in the interactive shell. Its commands are not
added to interactive history. Configure the filename or disable autoexec using
[menuconfig](configuration.md).

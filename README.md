# ZShell

A new shell for Zeal 8-bit OS, with history, batch processing, and a few other extras.

## Try it out

> [!NOTE]
> You can check out a live demo of ZShell + [coreutils](https://github.com/zoul0813/zeal-coreutils)
>
> [<kbd> <br> <br> &nbsp;&nbsp; Try it now! &nbsp;&nbsp; <br> <br> </kbd>](https://zoul0813.github.io/zeal-zshell/)


## Installation

You can either [build ZShell from source](#building-from-source), or use one of the [pre-built binaries](https://github.com/zoul0813/zeal-zshell/releases).


## Built-in commands

| Command | Description |
| ------- | ----------- |
| `#` | display the last return code |
| `cd` | change disk/directory |
| `clear` | clear the screen |
| `exit` | exit ZShell (when current shell, just restarts ZShell - __does not__ reset  system) |
| `false` | always returns false, used for batch scripts |
| `history` | show the command history (if enabled) |
| `pwd` | print the working directory |
| `reset` | soft reset the entire system |
| `set` | set [env vars](#environment-variables) |
| `true` | always returns true, used for batch scripts |
| `ver` | show the current ZShell version |
| `which` | show where a binary is located |

## ZScripts (batch processing)

ZShell supports batch scripts with a `.zs` extension, and will process these similar to
a DOS Batch script.

Example:

```text
echo Welcome to ZShell, the batch processor for Zeal OS
echo Lines starting with "?" will only run if the last command succeeded
echo Lines starting with ":" will run if the last command failed

? echo "The next line should say "Hello World"
? echo Hello World
: echo Goodbye World

cat a:/notafile
? echo Found "a:/notafile"
: echo Could not find "a:/notafile"

; The following should just exit because there are no error tests
cat a:/shouldfail
? echo "Did not fail"

; The following should just exit because there are no error tests
cat a:/willfaill
: echo "Failure expected"


echo "Test script complete"
```

ZScripts supports very simple logic, in the form of a ternary statement that falls thru.

The above script illustrates how the fall thru works:

* Lines starting with `?` will only execute if the previous command returned `ERR_SUCCESS` (ie; `0`).
* Lines starting with `:` will only execute if the previous command returned anything else (ie; `!0`).

This will fall thru, so if you have multiple lines starting with `?` they will each execute only if the
previous line was a success.

For `:` error statements, ... for example:

```text
cat a:/notafile
? echo "Will not display, because of ERR_NO_ENTRY error"
: echo "Will display, because of error"
: echo "Will not display, because previous line succeeded"
```

## Autoexec

ZShell can be configured to load an autoexec script at whatever path you prefer when compiling.

The default is `b:/autoexex.zs`

The purpose of this script is to set your initial `PATH` variable, but it can also be used to perform
common "boot" operations - such as changing your display font with [setfont](https://github.com/zoul0813/zeal-coreutils?tab=readme-ov-file#setfont),
using `cd t:/games` to change your default working directory, using `echo` to display a simple message, or even launching
your favorite game or application (ie; [Zeal Commander](https://github.com/zoul0813/zeal-commander))

## Environment Variables

ZShell has limited environment variable support, currently it only supports `PATH`.

For example, you can  `set PATH=A:/,B:/,T:/bin/`.

When `set` is run with just the var name `set PATH` it will display the vars values.

Programs __do not__ have access to these env vars, and they are not passed through.  These vars are internal
to ZShell only at the moment, but will eventually be accessible to ZScripts.

## Building from source

Make sure that you have [ZDE](https://github.com/zoul0813/zeal-dev-environment) installed.

Then open a terminal, go to the source directory and type the following commands:

```shell
zde cmake
```

### Menuconfig

ZShell supports a simple menuconfig, similar to the Zeal 8-bit OS kernel config.

```shell
zde cmake --target menuconfig
```

You can turn on/off various features of ZShell, such as [autoexec](#autoexec), command history, and color support.

You can also configure how much memory ZShell uses, and adjust the maximum number of PATH's, history entries, etc.


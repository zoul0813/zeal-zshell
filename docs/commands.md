# Built-in command reference

[Documentation index](README.md)

Built-ins execute inside ZShell. `cd`, `help`, and `set` are always included.
`history` follows `HISTORY_ENABLED`; every other command below has its own
`BUILTIN_*` option, enabled by default. Disabled commands disappear from `help`
and built-in lookup, allowing an external program of the same name to resolve.

A successful command returns status 0 (`ERR_SUCCESS`). Nonzero statuses indicate
failure and can be tested in [ZScripts](zscripts.md). Unless stated otherwise,
commands that take no parameters currently ignore extra argument text rather
than validating it.

| Command | Syntax | Purpose |
| --- | --- | --- |
| `#` | `#` | Print the previous status |
| `cd` | `cd path` | Change directory |
| `clear` | `clear` | Clear/reset the text screen |
| `exec` | `exec command [arguments]` | Dispatch another command through ZShell |
| `exit` | `exit` | Exit this shell process successfully |
| `false` | `false` | Return failure |
| `help` | `help` | List enabled built-ins |
| `history` | `history [clear]` | List or clear session history |
| `pwd` | `pwd` | Print the full current path |
| `reset` | `reset` | Restart through the system reset vector |
| `set` | `set [PATH[=directories]]` | Display or replace PATH |
| `true` | `true` | Return success |
| `ver` | `ver` | Print the application version |
| `which` | `which name` | Identify a built-in or find a file in PATH |

## `#`

Prints the preceding command's status as an unsigned decimal number followed by
a newline. It is a command, not a comment marker or variable expansion.

```text
false
#
```

This prints `1`. The `#` command itself returns success, so a subsequent `#`
prints `0`. In a script, use a conditional line to inspect a failure without
stopping at the next unconditional command:

```text
false
: #
```

## `cd`

```text
cd T:/games/
cd ..
cd A:/
```

Changes directory through the OS and refreshes the prompt's cached path. The
trailing slash is optional; ZShell appends it when needed. An empty argument
prints `cd <path>` and returns `ERR_INVALID_PARAMETER`. A failed directory
change returns the actual OS failure and does not report success. Directory
semantics such as `..` are supplied by the OS.

## `clear`

Resets the stdout text screen through `CMD_RESET_SCREEN`. It returns the ioctl
result. It does not clear history, change PATH, or delete disk files.

## `exec`

```text
exec ./scripts/demo.zs
exec echo Hello
```

Prints `exec 'command arguments'`, then dispatches the remaining text through the
same runner used at the prompt. This supports built-ins, binaries, and scripts;
it does not replace the shell process. Lookup, arguments, and return status
follow the invoked command's normal rules.

For a `.zs` script, additional argument text is rejected with
`ERR_INVALID_PARAMETER`. With no command, dispatch fails rather than opening a
new shell. Its diagnostic line still prints in a quiet script: quiet mode only
suppresses the batch processor's own command echo.

## `exit`

Restores the default keyboard mode and exits the current ZShell process with
status 0. It does not parse an exit-code argument. If ZShell is the system's
initial shell, the OS may restart it; this is not a full system reset. Inside a
ZScript it exits the shell process, rather than returning only from that script.

## `false`

Returns `ERR_FAILURE` (1) without output of its own. Interactive execution also
causes the shell's normal error report and feedback beep. Useful for testing
failure branches in scripts.

## `help`

Prints each enabled built-in name on its own line and returns success. It lists
commands, not per-command usage, external programs, or files in PATH.

## `history`

```text
history
history clear
```

With no arguments, prints the current session history in chronological order,
numbered from 1. `clear` removes every entry and resets history navigation.
Any other argument prints `usage: history [clear]` and returns
`ERR_INVALID_PARAMETER`. See [history behavior](interactive-shell.md#history)
for deduplication, draft restoration, and capacity limits.

## `pwd`

Prints the shell's complete current-directory path and returns success. Unlike
the prompt, its output does not abbreviate deep directory paths.

## `reset`

Executes the Z80 reset instruction `rst 0`, transferring control to the system
reset vector. Use it to soft-reset the guest system. The browser page's Reset
button instead reloads the emulator through its lifecycle API.

## `set`

```text
set
set PATH
set PATH=T:/apps/,T:/bin/,A:/,B:/
```

Only the exact uppercase variable name `PATH` is supported. With no arguments,
prints the environment in assignment form. `set PATH` prints indexed entries;
assignment validates and replaces the entire search list. Unknown variables and
invalid assignments return `ERR_INVALID_PARAMETER`. There is no append syntax,
unset command, variable expansion, or export to child programs.

For entry normalization, limits, and validation rules, see
[PATH](interactive-shell.md#path).

## `true`

Returns `ERR_SUCCESS` (0) with no output. Useful for script conditionals and
resetting the previous command status.

## `ver`

Prints `ZShell major.minor.patch`. The version is defined in the root
`CMakeLists.txt`. When `DEV_BUILD_NUMBER` is enabled, a hyphen and the local build
number are appended. Normal builds do not display a build-number suffix.

## `which`

```text
which cd
which echo
which demo.zs
which ./game.bin
```

For a built-in, prints `built-in: name`. Otherwise prints the resolved file path
using the same exact-name, `.bin`, then `.zs` search order as command execution.
`./name` bypasses built-ins and PATH and requires an exact filename. A failed
file search returns `ERR_NO_SUCH_ENTRY`, including cases where its underlying
lookup failed for another reason.

Unlike direct command execution, `which` does not special-case drive-qualified
absolute names: `which T:/bin/echo` is still processed through PATH. Use a bare
name to inspect PATH, or `./...` to check an exact current-directory file.
The entire argument string is treated as one search name, not multiple names.

# ZScripts

[Documentation index](README.md)

ZScripts are line-oriented batch files, conventionally named with a lowercase
`.zs` extension. Each executable line is sent to the same command runner as an
interactive command. Built-ins and external applications can be mixed freely.

## Run a script

```text
./scripts/demo.zs
exec ./scripts/demo.zs
zshell T:/scripts/demo.zs
zshell -q T:/scripts/demo.zs
```

The first two forms execute in the current shell and retain changes to PATH and
working directory. Bare script names can also resolve through PATH, including
`.zs` suffix guessing. Resolved `.zs` files run quietly by default. `exec` adds
its own diagnostic output.

Starting a separate `zshell` process with a script path runs it with command echo
enabled; `-q` suppresses that echo. It opens the supplied path directly and exits
with the script's final status. Startup script mode does not run autoexec.

Scripts do not receive arguments or positional parameters. For example,
`./demo.zs argument` is rejected with `ERR_INVALID_PARAMETER`. There is no
parameter substitution, local-variable declaration, or automatic script-directory
change. Relative paths are resolved from the current directory, not from the
location of the script file.

## Lines, whitespace, and comments

Use plain text with LF (`\n`) line endings. CRLF is not stripped by the parser;
a carriage return can become part of a filename or argument. A final newline is
optional, and an empty file succeeds.

Before interpreting a line, ZShell removes leading/trailing ASCII spaces and
collapses repeated spaces. Blank lines are ignored. If the first non-space
character is `;`, the whole line is a comment:

```text
; Launch a program from the community image.
cd T:/
echo Ready
```

Semicolons elsewhere do not start comments or separate commands. `#` is the
status command, not a comment marker. Quote characters have no special meaning
to the shell, and whitespace normalization also applies inside them.

The raw physical line must fit `COMMAND_MAX - 1` bytes, excluding LF: 255 bytes
with defaults. The limit is checked before whitespace normalization, even for
comments. Long files are streamed in 128-byte chunks and are not limited to one
read buffer. A line crossing a chunk boundary is assembled normally. An
oversized line aborts with `<line length>` and `ERR_PATH_TOO_LONG`; it is not
silently truncated or executed.

## Status conditionals

Each script starts with status 0. Every command that actually executes replaces
that script's status with its result. Blank lines, comments, and skipped
conditional lines leave it unchanged.

| Prefix | Execution rule |
| --- | --- |
| None | Execute unless a preceding unhandled failure stops the script |
| `?` | Execute only if the current script status is 0 |
| `:` | Execute only if the current script status is nonzero |

An optional space can follow `?` or `:`. These prefixes are script syntax, not
interactive operators. Conditional lines form a chain: they test the result of
the most recent command that actually ran, including earlier conditional lines.
They are not a paired `if`/`else` construct.

```text
true
? echo First success
: echo Runs only if the preceding echo fails
```

The failure line normally does not run because the first `echo` succeeds.

```text
false
? echo Skipped because false failed
: echo Failure handled
? echo Continues because the handler succeeded
```

The skipped success line preserves the failure status. The failure handler then
runs and, if successful, changes the status to 0.

## Stopping and continuing

A failed command followed by another unconditional command stops the script
before that next command runs:

```text
./missing.bin
echo Not reached
```

If one or more conditional lines follow the failure, the parser considers it a
conditional group. At the next unconditional line it resets the group's status
to success and continues, even if the last status in the group was a failure.
Blank lines and comments do not end the group.

```text
false
? echo Skipped
; The conditional group ends at the next command.
echo Reached
```

At end of file, no such reset occurs: the script returns its current status.
An unconditional failure at EOF therefore fails, as does a conditional chain
whose last executed command failed. This distinction matters when returning a
status from a nested script.

```text
; A child script that deliberately returns failure.
false
```

Use a conditional prefix for `#` after a failure, since an unconditional `#`
would otherwise be blocked by failure propagation:

```text
./missing.bin
: #
```

`#` prints the previous command status, then itself succeeds.

## Nested scripts

A script can call another with `exec ./child.zs` or directly as `./child.zs`.
Each active script has an independent line buffer and conditional status; the
parent resumes with the child's returned status. PATH and current-directory
changes are shared with the parent because execution remains in the same shell.
There is no automatic directory restoration on return.

`BATCH_MAX_DEPTH` limits active scripts, including the outermost script, to 4 by
default. The setting accepts 1–16. Opening another script at the limit prints
`<batch depth>` and returns `ERR_NO_MORE_MEMORY`; the parent can handle it with a
failure conditional. Autoexec counts toward this limit when it invokes scripts.
The `exit` built-in terminates the shell, not just one nesting level.

## Example startup script

```text
; Configure programs, select the TF disk, and show a welcome file.
set PATH=T:/apps/,T:/bin/,A:/,B:/
cd T:/
? acat B:/welcome.ans
: echo Could not show the welcome file
```

`echo` and `acat` must be installed external programs. Quiet mode suppresses only
ZShell's `> command` echo; it does not suppress program output, error messages,
or the `exec` diagnostic. Echoed conditional lines can be displayed even when
their command is skipped.

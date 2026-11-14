# Troubleshooting and limits

[Documentation index](README.md)

## Finding and running commands

| Symptom | What to check |
| --- | --- |
| `ERR_NO_SUCH_ENTRY` for a command | Inspect `set PATH`; use `which name`; use `./name.bin` for an exact current-directory file |
| Built-in absent from `help` | Check the matching `BUILTIN_*` setting; `history` depends on `HISTORY_ENABLED` |
| `ls`, `echo`, or `cat` missing | These are external programs; install the community ROM image or stage coreutils |
| Program with a `.bin` suffix loses to another file | Exact names are searched across all PATH entries before `.bin`, then `.zs` |
| `./game` fails but `game` works | Explicit paths do not guess extensions; use the exact filename |
| `which T:/...` fails | `which` only special-cases `./`; inspect with a PATH name or an exact `./` path |
| `set` rejects an assignment | Use uppercase `PATH`, no spaces around separators, and nonempty entries within configured limits |
| `cd` reports an error | Supply a directory path and ensure the drive is mounted; failure propagates as an OS error |
| Arguments change spacing or quotes | ZShell normalizes spaces globally and does not parse quoting; check the external program's argument rules |

Most [OS errors](../src/common.c) are printed interactively as
`ERROR($xx): ERR_NAME`, with the status in hexadecimal. Common statuses include:

| Decimal | Name | Typical cause |
| --- | --- | --- |
| 0 | `ERR_SUCCESS` | Successful command |
| 1 | `ERR_FAILURE` | `false`, or a general application failure |
| 4 | `ERR_NO_SUCH_ENTRY` | Missing executable, file, or directory |
| 6 | `ERR_INVALID_PARAMETER` | Invalid built-in syntax, script arguments, or startup option |
| 14 | `ERR_PATH_TOO_LONG` | Oversized path, dispatch argument, or physical script line |
| 18 | `ERR_READ_ONLY` | Write to a read-only filesystem |
| 22 | `ERR_NO_MORE_MEMORY` | Script-depth limit or OS memory exhaustion |
| 23 | `ERR_NOT_A_DIR` | A directory operation addressed a file |
| 24 | `ERR_NOT_A_FILE` | A file operation addressed another entry type |

Child programs can return other OS statuses. `#` prints the preceding command's
status in decimal, then succeeds itself. In a script use `: #` after a failure
so failure propagation does not stop the script before the status command runs.

## Editing and history

A full command buffer beeps instead of accepting another character. The default
limit is 255 characters, including spaces before normalization. A smaller
configured `COMMAND_MAX` lowers this limit. Character codes outside `0x20–0x7D`
are ignored during interactive insertion.

History does not survive shell exit. Duplicate commands move to the newest
position instead of creating new entries. Up/Down navigation stops at its ends;
editing a recalled command starts a new draft. If the keys and `history` command
are both unavailable, check `HISTORY_ENABLED`. In the browser, first click the
emulator screen to focus it.

## Scripts and startup

| Symptom | Explanation / action |
| --- | --- |
| `Could not load B:/autoexec.zs` | Install a regular file at the configured autoexec path, or change/disable autoexec; the shell still starts |
| Script ends after a failed command | An unhandled failure blocks the next unconditional line; use `?`/`:` intentionally |
| Both apparent branches run | Conditional lines test the latest executed status, not a fixed original result |
| Script continues after a failed conditional group | The next unconditional line resets the group's status; this is the batch parser's defined behavior |
| A nested script changes the parent directory/PATH | Scripts execute in the same shell and share those values |
| `<line length>` | A raw line exceeds `COMMAND_MAX - 1`; shorten it, including comment lines |
| `<batch depth>` | Too many active scripts; shorten the call chain or raise `BATCH_MAX_DEPTH` within the memory budget |
| Names contain unexpected characters | Save scripts with LF line endings; CRLF leaves carriage returns in the command |
| `.zs` invocation rejects arguments | Positional arguments are unsupported; use scripts without trailing argument text |
| Quiet script still prints text | Quiet suppresses batch command echo only, not application output or `exec` diagnostics |

An empty file and a final line without LF are supported. Blank/comment lines do
not change conditional status or break a conditional chain. See the
[ZScript reference](zscripts.md) for complete execution rules.

## Build failures

If Python cannot import `kconfiglib` or `menuconfig`, install the dependency in
the build environment's Python and rerun CMake configuration. If the generated
header reports an invalid numeric setting or storage budget, adjust the values
through menuconfig or reset to defaults. Do not edit `src/config.h` to hide the
problem. `ZOS_PATH`, SDK paths, compiler, and libraries must refer to the Zeal
build environment, not unrelated host installations.

Normal `ver` output has no local build number; this is intentional. Enable
`DEV_BUILD_NUMBER` only for builds where a changing local counter is wanted.

## Browser failures

The page's status area and developer console report runtime and storage errors.
Use HTTPS or localhost, a browser providing OPFS writable file streams and Web
Locks, and a profile that allows site storage. Close another emulator tab if it
already owns the workspace. Browser storage restrictions and quota errors have
no silent memory/IndexedDB fallback.

A deployment does not overwrite saved OPFS images. Use Update Images when it
appears to adopt changed images, or Clear Data to restore all deployed images.
Both ask before discarding local changes. Clear Data does not clear old
IndexedDB records. Unmute resumes audio when browser autoplay rules suspend it.

See [browser storage](deployment.md#browser-storage-and-compatibility) for
persistence boundaries, including buffered NOR changes.

## Regression examples

The repository's `tests/*.zs` files are guest-side regression examples, not an
automatically executed host test suite. With the repository's `tests` directory
available relative to the guest's current directory, and external programs such
as `echo` installed, run scripts explicitly, for example:

```text
./tests/nested.zs
./tests/status.zs
./tests/path.zs
./tests/cd.zs
./tests/root-path.zs
./tests/empty.zs
./tests/depth.zs
./tests/line-length.zs
./tests/startup.zs
```

The scripts cover nested parser state, statuses, PATH validation, directory
errors, empty files, depth rejection, oversized lines, and startup flags. Some
print diagnostic messages rather than checking every outcome automatically.
Depth/line-limit examples assume default settings; startup examples require a
`zshell` executable discoverable in PATH. PATH and directory tests change shell
state, so restore your preferred PATH and working directory afterward.

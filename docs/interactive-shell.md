# Interactive shell

[Documentation index](README.md)

## Prompt and feedback

The prompt displays the current drive and directory followed by `>`, for example
`T:/bin>`. For deep directories it abbreviates the middle with `/...` and retains
the last two path components. `pwd` always prints the full path.

With color support enabled, the prompt is light gray after success and dark red
after a failed command. Command text uses white on black. These colors belong to
the guest shell; the browser page palette does not change them.

Interactive command failures print `ERROR($xx): ERR_NAME`, where `xx` is the
hexadecimal OS status. A short beep sounds for reported command errors, a full
command buffer, Backspace at the start, or Delete at the end. The feedback uses
the VideoBoard sound hardware; there is no separate Kconfig sound switch.

## Keyboard editing

| Key | Action |
| --- | --- |
| Left / Right | Move one character within the current command |
| Home / End | Move to the beginning / end of the command |
| Backspace | Delete the character before the cursor |
| Delete | Delete the character under the cursor |
| Escape | Clear the command and reset history navigation |
| Enter | Normalize and execute a nonempty command |
| Up / Down | Navigate history when enabled |

Typing inserts at the cursor rather than overwriting. Editing and cursor motion
handle commands that wrap across screen lines. Home and End affect the command,
not the shell session. An empty Enter leaves the previous status unchanged.

The input buffer holds at most `COMMAND_MAX - 1` characters: 255 by default.
Interactive insertion accepts character codes `0x20` through `0x7D`; other input
is ignored. There is no completion, multiline command entry, or shell shortcut
such as Ctrl-C job control.

## Parsing and whitespace

Before execution and history storage, leading/trailing ASCII spaces are removed
and consecutive spaces collapse to one. This applies to the entire command,
including text inside quote characters. Tabs are not normalized into spaces.

The first space separates the command name from its argument string. The shell
does not implement quoting, escaping, wildcard expansion, variable substitution,
pipes, redirection, `&&`, or multiple commands on one line. An external program
may implement its own argument rules, but it receives the shell's normalized
argument text. Quotes typed into `echo "Hello"` are passed to `echo`; they are not
removed by ZShell.

Built-in names and shell variable names are matched exactly. Use lowercase
built-in names and uppercase `PATH`. Filesystem name matching is an OS/filesystem
property. Automatic script recognition specifically requires lowercase `.zs`.

## History

History is enabled by default with room for 10 distinct commands; both the
feature and capacity are [configurable](configuration.md).

- Up saves your current draft and recalls the newest history entry, then older
  entries. It stops at the oldest entry rather than wrapping.
- Down moves toward newer entries, then restores the saved draft. Further Down
  presses keep that draft.
- Inserting or deleting characters detaches the displayed command from history
  navigation. A subsequent Up begins a new navigation sequence with that edit as
  its draft. Cursor movement alone does not detach it.
- Repeating a normalized command moves its existing entry to the newest position;
  it does not create a duplicate. When full, a new command replaces the oldest.
- Commands are recorded before execution, including commands that fail. Scripts
  and autoexec do not populate interactive history.
- History is in RAM, lasts only for this shell session, and is not saved to disk.

`history` lists entries oldest first, numbered from 1. The `history` command itself
has already been recorded when its output is produced. `history clear` removes
all entries and the saved draft, including its own recorded entry.

## PATH

Only `PATH` is supported as a shell variable:

```text
set
set PATH
set PATH=T:/apps/,T:/bin/,A:/,B:/
```

`set` prints `PATH=...` in assignment form. `set PATH` prints one indexed path per
line, starting at index 0. Assignment replaces the entire list, in order.
Entries are comma-separated; a missing trailing `/` is added automatically.
Do not add spaces around `=` or commas. Prefer drive-qualified absolute entries
so lookup does not change when the current directory changes.

The default capacity is 8 entries, configurable from 1 to 16. Empty values,
empty entries, excess entries, extra `=`, and paths too long to store are rejected
before the active list is replaced. `set PATH=` cannot clear PATH. Assignment
checks syntax and size, not whether the directories exist.

PATH belongs to ZShell: it is neither exported to child programs nor persisted
between sessions. Put assignments in autoexec to restore them at startup.

## Command lookup

For an ordinary command such as `game`:

1. Match an enabled built-in first.
2. Search every PATH entry in order for the exact filename `game`.
3. If the supplied name has no filename extension, search every PATH entry for
   `game.bin`.
4. If still missing, search every PATH entry for `game.zs`.

Extension priority applies across the whole PATH: `B:/game` beats `A:/game.bin`
even when `A:/` appears first. Supplying an extension disables suffix guessing.
Directories are not executable matches. Missing entries allow lookup to
continue; other filesystem errors can stop the search.

The current directory is not searched implicitly. Use an explicit path:

```text
./game.bin
T:/games/game.bin
./scripts/demo.zs
```

A name beginning with `./` or a drive-qualified root such as `T:/` bypasses
built-ins and PATH, and must name the file exactly: no `.bin` or `.zs` suffix is
appended. A bare relative subpath such as `games/game` is searched beneath each
PATH entry; use `./games/game` to address the current directory explicitly.

A resolved `.zs` file runs within the current shell, quietly. Other files are
launched as OS child programs while preserving the shell, and their exit status
becomes the shell's status. Launch failures and keyboard-restoration errors also
propagate as failures. The shell restores its nonblocking raw keyboard mode when
a child returns.

See [`which`](commands.md#which) for lookup diagnostics and its explicit-path
limitation.

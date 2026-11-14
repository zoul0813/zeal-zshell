# Building and configuration

[Documentation index](README.md)

## Build with ZDE

Install and configure the
[Zeal Development Environment](https://github.com/zoul0813/zeal-dev-environment)
and its ZShell dependencies. The build uses SDCC, the Zeal OS CMake toolchain,
the Zeal VideoBoard SDK, and coreutils libraries. `ZOS_PATH` and `ZVB_SDK_PATH`
must point to the appropriate dependencies in the build environment. Python 3
and `kconfiglib` are required to generate configuration; interactive menus also
need its `menuconfig` module. Run dependency setup inside the ZDE environment
rather than assuming host libraries can be linked into a Zeal binary.

From the ZShell source directory:

```sh
zde cmake
zde cmake --target menuconfig
zde cmake
```

The first build creates defaults when `.config` is missing. The interactive
menu saves choices in `.config`; rebuild to apply them. Builds generate
`src/config.h` and a version header under the build directory's `generated/`
folder. These generated files should not be edited.

The resulting executable is `bin/zshell` (without a `.bin` extension). Linker
`.map` and `.ihx` files are moved to `debug/`. If root-level `autoexec.zs` or
`test.zs` exists during the build, it is copied to `bin/`. Building does not
create an autoexec file automatically or install the executable on a guest disk.

## Configuration targets

| Command | Effect |
| --- | --- |
| `zde cmake --target menuconfig` | Open the interactive configuration menu |
| `zde cmake --target config` | Alternate target for the menuconfig UI |
| `zde cmake --target oldconfig` | Load saved choices and write a configuration including new defaults |
| `zde cmake --target defconfig` | Replace existing choices with Kconfig defaults |
| `zde cmake --target config_clean` | Remove `.config`, `.config.old`, and `src/config.h` |
| `zde cmake --target zshell_help` | Show the available configuration/build targets |
| `zde cmake` | Build with the current configuration |

`defconfig` discards local choices. `config_clean` causes a later build to
regenerate defaults. The menu targets and `oldconfig` check for the Python
`menuconfig` module. If it is absent, install `kconfiglib` into the Python
environment used by the build, then reconfigure CMake so module detection runs
again. `oldconfig` merges defaults rather than asking a question for each symbol.

## All settings

Names below omit the `CONFIG_` prefix used in `.config`. Defaults are declared
in [Kconfig](../Kconfig).

| Setting | Default | Meaning / allowed values |
| --- | --- | --- |
| `AUTOEXEC_ENABLED` | `y` | Run a startup script during interactive startup |
| `AUTOEXEC_FILENAME` | `B:/autoexec.zs` | Startup path; available when autoexec is enabled |
| `HISTORY_ENABLED` | `y` | Include history storage, navigation, and the `history` command |
| `COLOR_SUPPORT` | `y` | Set guest text colors for prompt, input, and script echo |
| `COMMAND_MAX` | `256` | Command buffer bytes, 64–256; usable input is one byte less |
| `MAX_PATHS` | `8` | PATH entry capacity, 1–16 |
| `BATCH_MAX_DEPTH` | `4` | Simultaneously active scripts, 1–16 |
| `HISTORY_MAX_ENTRIES` | `10` | Distinct history entries, 1–32; available when history is enabled |
| `DEV_BUILD_NUMBER` | `n` | Increment and show a local development build number |
| `BUILTIN_HASH` | `y` | Include `#` |
| `BUILTIN_PWD` | `y` | Include `pwd` |
| `BUILTIN_EXIT` | `y` | Include `exit` |
| `BUILTIN_EXEC` | `y` | Include `exec` |
| `BUILTIN_CLEAR` | `y` | Include `clear` |
| `BUILTIN_WHICH` | `y` | Include `which` |
| `BUILTIN_TRUE` | `y` | Include `true` |
| `BUILTIN_FALSE` | `y` | Include `false` |
| `BUILTIN_VER` | `y` | Include `ver` |
| `BUILTIN_RESET` | `y` | Include `reset` |

`cd`, `help`, and `set` are always built. `history` has no separate built-in
switch. Disabling optional commands removes both their handlers from the binary
and their names from the built-in table. Disabling `exec` does not disable direct
`.zs` execution. Disabling history does not disable cursor editing.

Color support controls ZShell's guest text output, not the browser page's CSS or
the output of external programs. Error beeps remain enabled independently.

## Memory and path limits

The configuration generator validates numeric ranges and a combined configurable
storage estimate capped at 15,000 bytes:

```text
COMMAND_MAX
+ (HISTORY_MAX_ENTRIES × (COMMAND_MAX + 5), when history is enabled)
+ MAX_PATHS × 128
+ BATCH_MAX_DEPTH × COMMAND_MAX
```

This is a guard for these configured buffers, not an estimate of the entire
application's memory or a guarantee that every OS configuration has enough RAM.
Unsafe combinations fail configuration-header generation. Reduce history
capacity, nesting depth, or buffer sizes rather than bypassing the check.

The shipped OS configurations use a 128-byte path capacity; paths must leave
space for a terminating null byte. A directory PATH entry must also have room
for an appended slash when it is missing. Command and argument text are copied
separately into path-sized buffers, so a command line can fit the interactive
255-character limit yet contain an argument string too long to dispatch.

## Versioning

The application version is set in the root `CMakeLists.txt`. Normal builds use a
stable generated version header and do not increment a local counter. With
`DEV_BUILD_NUMBER=y`, building increments `build_number.txt` and appends its
value to `ver` output. This mode intentionally makes successive builds differ;
leave it disabled for reproducible release and CI builds. This option governs
ZShell's version header, not reproducibility of every dependency or OS image.

## Kernel configurations

`os.conf` and `emu.conf` are Zeal OS configurations, separate from ZShell's
`.config`. Both select ZShell as the initial executable and enable video,
keyboard, ROM disk, TF, CompactFlash, and ZealFS v2 support. `emu.conf` additionally
enables emulation HostFS in the kernel. Actual image contents and mounted devices
depend on the build; the browser page does not provide a HostFS mount control.

The [deployment guide](deployment.md#deployment-workflow) describes staging
coreutils and ZShell into the ROM disk and producing the demo's OS images.

# Browser demo and GitHub Pages deployment

[Documentation index](README.md)

The [live demo](https://zoul0813.github.io/zeal-zshell/) runs ZShell and coreutils
inside Zeal-NativeEmulator. Its webpage controls operate the emulator; commands
typed into its screen operate the guest OS and ZShell.

## Controls

Click the emulator screen to focus the keyboard. The page provides:

| Control | Behavior |
| --- | --- |
| Reset | Orderly emulator reload; persisted disks are retained, guest RAM is reset |
| Unmute | Resume suspended browser audio after a user gesture |
| Toggle FPS | Toggle the emulator frame-rate display |
| Clear Data | Ask before deleting configured OPFS disk images and restarting with deployed copies |
| Update Images | Appear when deployed images change; ask before replacing changed saved copies and restarting |

Clear Data and Update Images discard guest RAM and local changes to the affected
images. Clear Data restores all configured disks from the deployment. Updating
replaces only images detected as changed. Neither operation upgrades the guest
filesystem in place or merges local files into the downloaded images.

There is no Save Data button. Completed TF sector writes and EEPROM page writes
persist through the runtime's OPFS backend automatically. This covers writes
that reach the emulated device, not data an application still holds in its own
buffers. NOR/ROM changes have a different runtime policy: they remain buffered
until an orderly flush or exit; the page's Reset operation flushes before
teardown. Do not assume an abrupt tab close saves buffered NOR changes.

## Browser storage and compatibility

The page uses the emulator's `ZealNative` API with `imageStorage: 'opfs'`. Serve it
over HTTPS, or localhost for development. The runtime needs OPFS, writable file
streams (`FileSystemFileHandle.createWritable()`), and Web Locks. Modern Chromium
browsers and Firefox provide these storage APIs; Safari needs version 26 or newer
for writable file streams. API availability alone is not full emulator testing
on every browser. The deployment does not require cross-origin isolation,
SharedArrayBuffer, or custom GitHub Pages response headers.

On first use, the runtime downloads the deployed ROM, EEPROM, and TF images into
browser storage. Subsequent starts use saved images. Completed guest disk writes
stay local to that browser profile and origin; they do not change the hosted
files or another user's disks. The default storage workspace is scoped to the
page directory. Another tab cannot own the same workspace concurrently.

The old IndexedDB records are left intact and are not migrated or deleted by
OPFS Clear Data. There is no automatic fallback to IndexedDB or memory mode.
Browser site-data controls can delete stored disks, and quota or storage-policy
restrictions can cause failures. ZShell's command history is separate and remains
session-only even though disk images persist.

Startup checks remote image validators in the background. Changed ETag or
Last-Modified values expose Update Images without silently replacing saved
files. This includes a new TF image or a rebuilt ROM/EEPROM. Downloads and update
errors are reported in the page status area and browser console. If validators
are unavailable, automatic update detection is unavailable; local disks remain
intact.

## Deployment workflow

[`.github/workflows/build.yml`](../.github/workflows/build.yml) runs on pushes to
`main` and manual `workflow_dispatch`. It:

1. Sets up ZDE with the `zealfs` and `zshell` dependencies and checks out ZShell.
2. Checks out `Zeal8bit/Zeal-NativeEmulator` at `feature/wasm-opfs`.
3. Uses Emscripten 7.0.0, Meson, and Ninja to build the minimal WASM template.
4. Copies the matching WASM runtime, browser libraries, and ZShell logo into
   `.deploy` using `setup.sh`.
5. Downloads the latest published community release's `tf-2mib.img.zip` from
   [Zeal Software Collection](https://github.com/Zeal8bit/Zeal-Software-Collection/releases/latest).
   It extracts the image and checksum, checks SHA-256 and the exact 2,097,152-byte
   size, then installs it as `.deploy/tf.img`. The original ZIP, including licenses
   and notices, is also included as `community-image.zip`.
6. Stages coreutils, builds ZShell, and adds `bin/zshell` to the ROM disk.
7. Creates a 64 KiB EEPROM image from `.deploy/eeprom`, including the demo's
   autoexec and welcome text.
8. Builds the kernel/ROM disk using `emu.conf` and installs it as `default.img`.
9. Uploads `.deploy` and deploys it using the repository's Pages action.

The community image is resolved afresh for every workflow run, rather than
pinned to a release tag or downloaded by each visitor. A later release appears
in the demo only after another successful deployment. Saved browser disks still
require explicit update or clearing before they adopt deployed changes.

No TF image or compiled emulator asset is tracked in Git. The workflow builds
ZShell's own ROM/EEPROM and uses the community release for the TF disk. A failed
download, checksum, size check, or build prevents the upload/deploy steps.

## Demo startup

The demo's [autoexec](../.deploy/eeprom/autoexec.zs) sets:

```text
set PATH=T:/apps/,T:/bin/,A:/,B:/
cd T:/
acat b:/welcome.ans
```

The welcome file is a 78-column, 16-row CP437 banner with a half-block version
of the approved Z mark beside the welcome text. It fits within the default
80-column, 40-row display. Use `acat`, which interprets its ANSI color sequences;
plain `cat` does not render ANSI. It uses the default 16-color palette's magenta,
bright magenta, cyan, and gray on black, rather than unsupported true-color
escape sequences. A custom font must retain CP437 block and box-drawing glyphs.

The editable source is [`scripts/generate_welcome.py`](../scripts/generate_welcome.py).
Regenerate the checked-in CP437 file after editing its artwork or text:

```sh
python3 scripts/generate_welcome.py
```

Do not save `.deploy/eeprom/welcome.ans` as UTF-8: its glyphs must remain single
CP437 bytes. The file uses CRLF for `acat` cursor alignment; ZScript source files
such as `autoexec.zs` still use LF line endings. The banner restores normal text
colors before returning to the prompt and does not clear existing boot output.

The set of programs on `T:` follows the selected community release. Commands
mentioned by the welcome text are external programs and depend on that image.
The kernel enables emulation HostFS, but the page does not expose a HostFS mount
button.

## Local assets and branding

Build the emulator's OPFS branch with the pinned Emscripten version and the
`wasm-cross.build` cross file. From the ZShell repository, copy a completed build:

```sh
bash .deploy/setup.sh /path/to/Zeal-NativeEmulator/build
```

Supply `default.img`, `eeprom.img`, and `tf.img` in `.deploy`, then serve that
directory through localhost or HTTPS. Do not open `index.html` as a `file:` URL.
The runtime and its `libnative` scripts must come from the same build; mixing
these with the older IndexedDB runtime is unsupported.

The editable logo source is [`docs/images/logo.svg`](images/logo.svg). README
uses that file directly; `setup.sh` copies it to `.deploy/logo.svg` for Pages.
The SVG has path-based lettering, Zeal purple on light backgrounds, and lavender
on dark backgrounds. The page sizes it in CSS and retains a `+ coreutils` label.
The page palette follows the light/dark colors used by the
[Zeal Pascal book](https://zoul0813.github.io/zeal-pascal/book/).

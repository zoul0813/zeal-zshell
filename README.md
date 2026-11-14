<img src="docs/images/logo.svg" alt="ZShell" width="480">

A new shell for Zeal 8-bit OS, with history, batch processing, and a few other extras.

## Try it out

> [!NOTE]
> You can check out a live demo of ZShell + [coreutils](https://github.com/zoul0813/zeal-coreutils)
>
> [<kbd> <br> <br> &nbsp;&nbsp; Try it now! &nbsp;&nbsp; <br> <br> </kbd>](https://zoul0813.github.io/zeal-zshell/)

## Documentation

The [ZShell documentation](docs/README.md) covers all shell features and the browser demo.

- [Getting started and startup options](docs/getting-started.md)
- [Interactive editing, history, command lookup, and PATH](docs/interactive-shell.md)
- [Built-in command reference](docs/commands.md)
- [ZScript syntax, conditionals, and examples](docs/zscripts.md)
- [Building and configuration reference](docs/configuration.md)
- [Browser demo and GitHub Pages deployment](docs/deployment.md)
- [Troubleshooting and limits](docs/troubleshooting.md)

## Installation

Download and extract the latest
[Zeal Software Collection community TF image](https://github.com/Zeal8bit/Zeal-Software-Collection/releases/latest).
Write it to your TF card (replacing its existing contents), or attach it in your
emulator, and boot with the image mounted as `T:`. Install ZShell and coreutils
onto the ROM disk with:

```text
t:/sys/flasher t:/sys/zshell.rom
```

Reset after flashing completes successfully. See the
[installation guide](docs/getting-started.md) for details, or
[build from source](docs/configuration.md) for a custom configuration.

## Building from source

With [ZDE](https://github.com/zoul0813/zeal-dev-environment) and ZShell's
dependencies installed:

```sh
zde cmake
```

To configure features before rebuilding:

```sh
zde cmake --target menuconfig
zde cmake
```

See the [build and configuration reference](docs/configuration.md) for every
setting, default, memory limit, and generated output.

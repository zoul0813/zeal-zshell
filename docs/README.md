<img src="images/logo.svg" alt="ZShell" width="480">

# ZShell documentation

ZShell is an interactive shell and ZScript batch processor for the Zeal 8-bit OS.
It provides command-line editing, session history, configurable executable lookup,
startup scripts, built-in commands, status reporting, and optional color feedback.

These pages describe the implementation in this repository. Features marked
optional depend on the build configuration; `help` lists the built-ins actually
present in the running shell. Examples assume the default configuration unless
stated otherwise.

| Guide | Contents |
| --- | --- |
| [Getting started](getting-started.md) | Installation, drives, first commands, startup scripts, command-line options |
| [Interactive shell](interactive-shell.md) | Prompt, keyboard editing, history, command parsing, PATH, lookup order |
| [Built-in commands](commands.md) | Syntax, effects, return values, and examples for every built-in |
| [ZScripts](zscripts.md) | Script execution, comments, quiet mode, status conditionals, nesting, limitations |
| [Build and configuration](configuration.md) | Dependencies, build outputs, configuration targets, every Kconfig setting |
| [Browser demo and deployment](deployment.md) | Page controls, OPFS storage, image updates, community TF image, deployment workflow |
| [Troubleshooting](troubleshooting.md) | Common failures, error messages, limits, and regression scripts |

## Quick example

On a system with coreutils installed in `A:/` and community applications on `T:/`:

```text
set PATH=T:/apps/,T:/bin/,A:/,B:/
cd T:/
pwd
help
which echo
history
```

`echo`, `cat`, `ls`, `tree`, and other filesystem utilities are separate programs,
not ZShell built-ins. Install them separately or use the
[browser demo](https://zoul0813.github.io/zeal-zshell/), which includes coreutils.

## Project resources

- [Project README](../README.md)
- [Release downloads](https://github.com/zoul0813/zeal-zshell/releases)
- [Changelog](../CHANGELOG)
- [License](../LICENSE)

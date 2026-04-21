# TIZEN-SOUND-POOL-CLI(1)

## NAME
**tizen-sound-pool-cli** - Exercise finite Tizen Sound Pool loading,
state, and playback flows.

## SYNOPSIS
**tizen-sound-pool-cli** *COMMAND* [*OPTIONS*]  
**tizen-sound-pool-cli** **-h** | **--help** | **help**  
**tizen-sound-pool-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-sound-pool-cli** provides command-line access to the public
Sound Pool APIs from `sound_pool.h`.

The tool focuses on finite one-shot flows that fit a non-persistent CLI:
- creating a temporary sound pool and inspecting its state
- activating or deactivating a temporary pool
- querying or setting the global pool volume
- loading and unloading one tagged source file
- loading, activating, playing, inspecting, and stopping one tagged
  source file in a single invocation

The underlying Sound Pool APIs are deprecated since Tizen 10.0, but they
remain the real public APIs exposed by this CLI.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **state** | None | Creates a sound pool and prints the initial pool state from `sound_pool_get_state()`. |
| **activate-demo** | None | Creates a pool, activates it, and prints the resulting state. |
| **deactivate-demo** | None | Creates a pool, activates it, deactivates it, and prints the resulting state. |
| **pool-volume** | `[--value <0.0-1.0>]` | Gets the current global pool volume, or sets it first and then reads it back. |
| **load** | `--path <PATH> --tag <TAG>` | Loads one source file into a temporary pool with `sound_pool_load_source_from_file()`. |
| **unload-demo** | `--path <PATH> --tag <TAG>` | Loads one source file and then unloads it with `sound_pool_unload_source()`. |
| **play-file** | `--path <PATH> --tag <TAG>` | Loads one source file, activates the pool, starts playback, reads the stream state, and stops the stream. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or Sound Pool API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-sound-pool-cli`

## AUTHOR
Part of the **Tizen Tools** project.

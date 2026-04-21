# TIZEN-PLAYER-CLI(1)

## NAME
**tizen-player-cli** - Exercise public Tizen Player APIs from a
non-interactive shell.

## SYNOPSIS
**tizen-player-cli** *COMMAND* [*OPTIONS*]  
**tizen-player-cli** **-h** | **--help** | **help**  
**tizen-player-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-player-cli** provides command-line access to the public Player API
from `player.h`.

The tool focuses on finite player lifecycle and query operations that are
practical in shell-driven validation:
- creating a player and reporting its state
- setting and reading instance volume
- toggling and querying the looping flag
- preparing a URI and printing playback metadata
- querying audio/text track counts for prepared media
- starting a prepared URI briefly and stopping it again

URI-based commands rely on the normal Player privileges described in
`player.h`, including `mediastorage`, `externalstorage`, or `internet`
depending on where the media is located.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **state** | None | Creates a player handle and prints the initial player state from `player_get_state()`. |
| **volume** | `<LEFT> <RIGHT>` | Sets and reads back the player instance volume with `player_set_volume()` and `player_get_volume()`. |
| **set-looping** | `--value <on\|off>` | Sets the looping flag on a new player instance with `player_set_looping()`. |
| **looping-status** | None | Prints the default looping flag from `player_is_looping()`. |
| **info** | `--uri <URI>` | Prepares the URI and prints state, duration, current position, volume, and looping status. |
| **track-counts** | `--uri <URI>` | Prepares the URI and prints audio and text track counts from `player_get_track_count()`. |
| **play** | `--uri <URI>` | Prepares the URI, starts playback, prints sampled state and position, then stops playback. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or Player API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-player-cli`

## AUTHOR
Part of the **Tizen Tools** project.

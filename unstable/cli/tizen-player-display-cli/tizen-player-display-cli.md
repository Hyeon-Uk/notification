# TIZEN-PLAYER-DISPLAY-CLI(1)

## NAME
**tizen-player-display-cli** - Exercise public Tizen Player display APIs from
a non-interactive shell.

## SYNOPSIS
**tizen-player-display-cli** *COMMAND* [*OPTIONS*]  
**tizen-player-display-cli** **-h** | **--help** | **help**  
**tizen-player-display-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-player-display-cli** provides command-line access to the display and
display-adjacent control APIs from `player.h`.

The tool focuses on finite commands that work well in shell-driven validation:
- setting and reading back display mode
- setting and reading back display rotation
- setting and reading back display visibility
- reading the current video size
- setting and querying the audio-only flag

The CLI creates a fresh player instance per command and attempts to use
`PLAYER_DISPLAY_TYPE_NONE` so display-related getters and setters can be
exercised in headless-style environments.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **set-get-mode** | `--mode <0\|1\|2\|3\|4\|5>` | Sets the display mode with `player_set_display_mode()` and prints the resolved mode from `player_get_display_mode()`. |
| **set-get-rotation** | `--rotation <0\|90\|180\|270>` | Sets the display rotation with `player_set_display_rotation()` and prints the resolved rotation from `player_get_display_rotation()`. |
| **set-get-visible** | `--visible <true\|false>` | Sets the display visibility with `player_set_display_visible()` and prints the resolved visibility from `player_is_display_visible()`. |
| **get-video-size** | None | Prints the current width and height from `player_get_video_size()`. |
| **set-audio-only** | `--value <on\|off>` | Sets the audio-only flag with `player_set_audio_only()` and prints the resolved value from `player_is_audio_only()`. |
| **audio-only-status** | None | Prints the current audio-only flag from `player_is_audio_only()`. |

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
`/opt/usr/home/owner/.local/bin/tizen-player-display-cli`

## AUTHOR
Part of the **Tizen Tools** project.

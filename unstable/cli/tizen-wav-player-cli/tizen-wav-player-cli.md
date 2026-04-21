# TIZEN-WAV-PLAYER-CLI(1)

## NAME
**tizen-wav-player-cli** - exercise the public Tizen WAV player API from the command line

## SYNOPSIS
**tizen-wav-player-cli** <command> [arguments]

## DESCRIPTION
**tizen-wav-player-cli** is a small wrapper around the public
`wav_player_start_new()`, `wav_player_start_loop()`, and `wav_player_stop()`
APIs.

The tool creates a temporary `sound_stream_info_h` handle through
`sound_manager_create_stream_information()` when starting playback. Commands are
finite and return immediately after invoking the native API.

## COMMANDS REFERENCE

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `stream-types` | none | List the public sound stream types accepted by this CLI. |
| `start-new` | `<path> [stream-type]` | Create stream information and call `wav_player_start_new()`. |
| `start-loop` | `<path> <loop-count> [stream-type]` | Create stream information and call `wav_player_start_loop()`. |
| `start-stop` | `<path> [stream-type]` | Start playback and immediately call `wav_player_stop()` with the returned player ID. |
| `stop` | `<id>` | Call `wav_player_stop()` for an existing WAV player ID. |
| `help`, `--help`, `-h` | none | Show usage help. |
| `version`, `--version`, `-V` | none | Show tool version. |

Supported `stream-type` values:
`media`, `system`, `alarm`, `notification`, `emergency`,
`voice-information`, `voice-recognition`, `ringtone-voip`, `voip`,
`media-external-only`.

## EXIT STATUS
`0` for success, `1` for invalid input or native API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-wav-player-cli`

## AUTHOR
Part of the **Tizen Tools** project.

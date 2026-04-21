# TIZEN-SCREEN-MIRRORING-CLI(1)

## NAME
**tizen-screen-mirroring-cli** - Configure and inspect a Tizen screen mirroring sink.

## SYNOPSIS
**tizen-screen-mirroring-cli** *COMMAND* [*OPTIONS*]  
**tizen-screen-mirroring-cli** **-h** | **--help** | **help**  
**tizen-screen-mirroring-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-screen-mirroring-cli** provides command-line access to the public
screen mirroring sink APIs from `scmirroring_sink.h` and
`scmirroring_type.h`.

The tool supports:
- creating a sink handle and printing its current state
- configuring sink network endpoint and resolution before prepare
- setting display mode, rotation, and custom ROI on a fresh sink handle

Commands may return `not supported` or permission-related errors on
devices that do not expose Wi-Fi Direct Display features.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **state** | None | Standard | Creates a sink handle and prints the current sink state. |
| **setup-sink** | `--ip <IP> --port <PORT> --resolution <MASK>` | Platform dependent | Creates a sink, sets IP/port and resolution, prepares it, and prints the resulting state. |
| **set-display-mode** | `--mode <0|1|2|3|4|5>` | Platform dependent | Sets the sink display mode on a fresh sink handle. |
| **set-display-rotation** | `--rotation <0|90|180|270>` | Platform dependent | Sets the sink display rotation on a fresh sink handle. |
| **set-display-roi** | `--x <X> --y <Y> --width <W> --height <H>` | Platform dependent | Sets a custom destination ROI on a fresh sink handle. |

Display mode values:
- `0`: letter-box
- `1`: origin-size
- `2`: full-screen
- `3`: cropped-full
- `4`: origin-or-letter
- `5`: custom-roi

Common resolution masks:
- `1`: `1920x1080p30`
- `2`: `1280x720p30`
- `4`: `960x540p30`
- `8`: `864x480p30`
- `16`: `720x480p60`
- `32`: `640x480p60`
- `64`: `640x360p30`

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or screen mirroring API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-screen-mirroring-cli`

## AUTHOR
Part of the **Tizen Tools** project.

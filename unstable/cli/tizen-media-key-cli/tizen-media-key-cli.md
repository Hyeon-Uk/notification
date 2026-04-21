# TIZEN-MEDIA-KEY-CLI(1)

## NAME
**tizen-media-key-cli** - Reserve Tizen media key events and inspect the
public media key enums.

## SYNOPSIS
**tizen-media-key-cli** *COMMAND* [*OPTIONS*]  
**tizen-media-key-cli** **-h** | **--help** | **help**  
**tizen-media-key-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-media-key-cli** provides command-line access to the public
Tizen media key API from `media_key.h`.

The tool supports:
- reserving media key events and printing callback payloads to stdout
- releasing an existing media key reservation
- listing the public `media_key_e` and `media_key_event_e` values used by
  the API

The `reserve` command runs the native `Ecore` main loop. For automated
validation, `--timeout <SECS>` can be used to exit the loop cleanly after
the requested interval.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **reserve** | `[--timeout <SECS>]` | Calls `media_key_reserve()` and prints media key callback events as JSON lines. |
| **release** | None | Calls `media_key_release()` and reports whether the reservation was detached or already unavailable. |
| **list-keys** | None | Prints the public `media_key_e` values and their integer codes. |
| **list-statuses** | None | Prints the public `media_key_event_e` values and their integer codes. |

## EXIT STATUS
**0**: Success, or a documented runtime warning such as unavailable media
key service.  
**1**: Invalid arguments, invalid timeout input, or `Ecore`
initialization failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-media-key-cli`

## AUTHOR
Part of the **Tizen Tools** project.

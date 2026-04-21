# TIZEN-SOUND-MANAGER-CLI(1)

## NAME
**tizen-sound-manager-cli** - Query Tizen sound-manager volume state and
connected audio devices.

## SYNOPSIS
**tizen-sound-manager-cli** *COMMAND* [*OPTIONS*]  
**tizen-sound-manager-cli** **-h** | **--help** | **help**  
**tizen-sound-manager-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-sound-manager-cli** provides command-line access to the public
Tizen Sound Manager APIs from `sound_manager.h`.

The tool supports:
- querying current and maximum volume for public `sound_type_e` values
- setting a volume value within the API-reported valid range
- querying the currently playing sound type
- listing connected sound devices and their direction, type, id, and
  running state
- listing the public sound types exposed by this CLI

The `set-volume` command requires the public privilege
`http://tizen.org/privilege/volume.set`.

`current-type` can fail when no sound is actively playing, and
`list-devices` can report `[WARN]` when the platform reports no matching
connected devices.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **get-volume** | `--type <TYPE>` | Standard | Prints the current and maximum volume for the requested sound type using `sound_manager_get_volume()` and `sound_manager_get_max_volume()`. |
| **set-volume** | `--type <TYPE> --value <LEVEL>` | `http://tizen.org/privilege/volume.set` | Sets the volume after validating the requested level against the API-reported maximum. |
| **current-type** | None | Standard | Prints the sound type returned by `sound_manager_get_current_sound_type()`. |
| **list-devices** | `[--direction <all|input|output>]` | Standard | Lists connected devices from `sound_manager_get_device_list()` and prints per-device id, name, type, direction, and running state. |
| **list-types** | None | Standard | Prints the public `sound_type_e` values supported by this CLI. |

Supported sound types:
- `system`
- `notification`
- `alarm`
- `ringtone`
- `media`
- `call`
- `voip`
- `voice`

## EXIT STATUS
**0**: Success, or a documented runtime fallback reported with `[WARN]`.  
**1**: Invalid arguments or Sound Manager API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-sound-manager-cli`

## AUTHOR
Part of the **Tizen Tools** project.

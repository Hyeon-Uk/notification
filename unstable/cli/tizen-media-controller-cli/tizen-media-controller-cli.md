# TIZEN-MEDIA-CONTROLLER-CLI(1)

## NAME
**tizen-media-controller-cli** - Query the latest Tizen media controller
server and inspect published client-side state.

## SYNOPSIS
**tizen-media-controller-cli** *COMMAND* [*OPTIONS*]  
**tizen-media-controller-cli** **-h** | **--help** | **help**  
**tizen-media-controller-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-media-controller-cli** provides command-line access to the public
Tizen media controller client APIs from `media_controller_client.h`,
`media_controller_metadata.h`, and `media_controller_type.h`.

The tool supports:
- resolving the latest active media controller server
- listing activated media controller servers
- inspecting playback state and position
- retrieving published metadata, shuffle mode, repeat mode, and icon URI
- inspecting playback action support exposed by the server

For query commands, omitting `SERVER_NAME` makes the CLI resolve the
latest active server automatically. If no active server is present, the
tool prints a documented `[WARN]` line and exits successfully.

Most runtime commands require the public privilege
`http://tizen.org/privilege/mediacontroller.client`.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **latest** | None | Prints the latest known media controller server and its activation state from `mc_client_get_latest_server_info()`. |
| **list-servers** | None | Lists currently activated media controller servers from `mc_client_foreach_server()`. |
| **playback** | `[SERVER_NAME]` or `--server <NAME>` | Prints playback state and playback position in milliseconds using `mc_client_get_server_playback_info()`. |
| **metadata** | `[SERVER_NAME]` or `--server <NAME>` | Prints published metadata fields retrieved from `mc_client_get_server_metadata()` and `mc_metadata_get()`. |
| **shuffle-mode** | `[SERVER_NAME]` or `--server <NAME>` | Prints the latest shuffle mode from `mc_client_get_server_shuffle_mode()`. |
| **repeat-mode** | `[SERVER_NAME]` or `--server <NAME>` | Prints the latest repeat mode from `mc_client_get_server_repeat_mode()`. |
| **icon** | `[SERVER_NAME]` or `--server <NAME>` | Prints the icon URI returned by `mc_client_get_server_icon()`. |
| **playback-ability** | `[SERVER_NAME]` or `--server <NAME>` | Prints per-action playback support from `mc_client_get_server_playback_ability()` and `mc_playback_action_is_supported()`. |

## EXIT STATUS
**0**: Success, or a documented runtime warning such as no active server
or unavailable published data.  
**1**: Invalid arguments or malformed input such as an embedded NUL byte
in `SERVER_NAME`.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-media-controller-cli`

## AUTHOR
Part of the **Tizen Tools** project.

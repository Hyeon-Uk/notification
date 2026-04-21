# TIZEN-WIFI-DIRECT-CLI(1)

## NAME
**tizen-wifi-direct-cli** - inspect public Wi-Fi Direct state, WPS, and local metadata on Tizen.

## SYNOPSIS
**tizen-wifi-direct-cli** *COMMAND* [*OPTIONS*]  
**tizen-wifi-direct-cli** **-h** | **--help** | **help**  
**tizen-wifi-direct-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-wifi-direct-cli** is a one-shot command-line interface for the public
Tizen Wi-Fi Direct API in `wifi-direct.h`.

The tool initializes Wi-Fi Direct, performs a single query command, prints the
result, and then deinitializes the client when appropriate. Commands that
depend on platform support or privileges can return a bounded `Error:` line on
devices or emulators where Wi-Fi Direct is unavailable.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **state** | None | Standard | Prints the current Wi-Fi Direct state returned by `wifi_direct_get_state()`. |
| **state-values** | None | Standard | Lists the public `wifi_direct_state_e` values compiled into the tool. |
| **wps-types** | None | Standard | Lists the public `wifi_direct_wps_type_e` values compiled into the tool. |
| **wps-info** | None | Standard | Prints local and requested WPS configuration values from `wifi_direct_get_local_wps_type()` and `wifi_direct_get_req_wps_type()`. |
| **discovery-flags** | None | Standard | Prints discoverable and listening-only flags returned by `wifi_direct_is_discoverable()` and `wifi_direct_is_listening_only()`. |
| **session-timer** | None | Standard | Prints the configured local session timer in seconds. |
| **set-session-timer** | `--seconds <SECONDS>` | Standard | Sets the local session timer and reads it back with `wifi_direct_get_session_timer()`. |
| **req-wps-roundtrip** | `--type <pbc\|pin-display\|pin-keypad>` | Standard | Sets the requested WPS type and reads it back with `wifi_direct_get_req_wps_type()`. |
| **device-info** | None | Standard | Prints local device name, MAC address, primary device type, and secondary device type. |
| **network-info** | None | Standard | Prints SSID, network interface name, IP address, subnet mask, gateway address, and operating channel. |
| **connection-info** | None | Standard | Prints group owner intent and maximum client count. |

## EXIT STATUS
**0**: Success.

**1**: Invalid arguments or runtime failure from the Wi-Fi Direct API.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-wifi-direct-cli`

## AUTHOR
Part of the Tizen Tools project.

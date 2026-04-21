# TIZEN-WIFI-MANAGER-CLI(1)

## NAME
**tizen-wifi-manager-cli** - query public Wi-Fi manager state and connected AP metadata on Tizen

## SYNOPSIS
**tizen-wifi-manager-cli** *command*

## DESCRIPTION
**tizen-wifi-manager-cli** is a one-shot command-line interface for the public
Tizen Wi-Fi Manager API in `wifi-manager.h`.

The tool initializes the Wi-Fi manager, performs a single query, prints the
result, and then deinitializes the manager handle. On unsupported devices or
shell contexts without the required privileges, runtime commands return a
bounded `Error:` line instead of hanging.

## COMMANDS REFERENCE

| Command | Description |
| --- | --- |
| **status** | Print activation, module, connection, and scan status. |
| **device-states** | List the public `wifi_manager_device_state_e` values. |
| **connection-states** | List the public `wifi_manager_connection_state_e` values. |
| **scan-states** | List the public `wifi_manager_scan_state_e` values. |
| **module-states** | List the public `wifi_manager_module_state_e` values. |
| **band-support** | Print whether 5 GHz and 6 GHz Wi-Fi bands are supported. |
| **ip-conflict-states** | List the public `wifi_manager_ip_conflict_state_e` values. |
| **ip-conflict-status** | Print IP conflict detection enablement and current conflict status. |
| **wps-generated-pin** | Print the generated WPS PIN from the current Wi-Fi manager session. |
| **local-info** | Print the local MAC address and network interface name. |
| **connected-ap-summary** | Print a summary of the currently connected access point. |
| **connected-ap-network** | Print IPv4 addressing details for the currently connected access point. |
| **connected-ap-security** | Print security, encryption, passphrase, and WPS details for the connected access point. |
| **help**, **--help**, **-h** | Show command help. |
| **version**, **--version**, **-V** | Show the tool version. |

## EXIT STATUS
**0**: Success.

**1**: Invalid arguments or runtime failure from the Wi-Fi manager API.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-wifi-manager-cli`

## AUTHOR
Part of the Tizen Tools project.

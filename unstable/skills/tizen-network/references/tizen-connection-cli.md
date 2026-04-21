# TIZEN-CONNECTION-CLI(1)

## NAME
**tizen-connection-cli** - Query current connection state and related network metadata.

## SYNOPSIS
**tizen-connection-cli** *COMMAND* [*OPTIONS*]  
**tizen-connection-cli** **-h** | **--help** | **help**  
**tizen-connection-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-connection-cli** provides one-shot access to the public connection
manager APIs from `net_connection.h` and selected non-deprecated extension
queries from `connection_extension.h`.

The tool focuses on read-only inspection of the active network state:
connection type, IP and proxy addresses, per-interface state, DHCP state,
Ethernet cable state, metered status, and NTP-related status.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **type** | None | `network.get` | Prints the active connection type. |
| **ip** | None | `network.get` | Prints the active IPv4 and IPv6 addresses. |
| **proxy** | None | `network.get` | Prints the active IPv4 and IPv6 proxy addresses. |
| **mac** | `--interface <wifi\|ethernet>` | `network.get` | Prints the Wi-Fi or Ethernet MAC address. |
| **state** | `--interface <wifi\|cellular\|ethernet\|bluetooth>` | Varies by interface | Prints the state of the selected interface. |
| **metered** | None | `network.get` | Prints whether the active connection is metered. |
| **dhcp-state** | None | Standard | Prints the DHCP state reported by the connection manager. |
| **ethernet-cable** | None | `network.get` | Prints whether an Ethernet cable is attached. |
| **clock-updated** | None | Platform dependent | Prints whether NTP updated the system clock. |
| **get-ntp-server** | None | Platform dependent | Prints the configured NTP server if one is present. |
| **summary** | None | Mixed | Prints a one-shot summary combining the above queries. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, API failure, permission failure, or unavailable
runtime state.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-connection-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-INTELLIGENT-NETWORK-MONITORING-CLI(1)

## NAME
**tizen-intelligent-network-monitoring-cli** - Inspect Intelligent
Network Monitoring state and connection diagnostics.

## SYNOPSIS
**tizen-intelligent-network-monitoring-cli** *COMMAND* [*ARGS*]  
**tizen-intelligent-network-monitoring-cli** **-h** | **--help** | **help**  
**tizen-intelligent-network-monitoring-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-intelligent-network-monitoring-cli** provides one-shot wrappers over the
public Intelligent Network Monitoring APIs from `inm.h` and selected
non-deprecated extension queries from `inm-extension.h`.

The tool focuses on read-only diagnostics: subsystem state, traffic
statistics, congestion level, retry/transmission health, channel interference,
IP-conflict status, current connection metadata, and tcpdump runtime status.

Runtime commands require `inm_initialize()` to succeed. On emulators or
headless environments, some queries can be unavailable when the INM daemon is
not running or when a metric is not supported by the current network stack.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **state** | `<wifi\|ethernet\|cellular\|gateway\|dns>` | Prints a single INM subsystem state. |
| **status** | None | Prints a one-shot summary of major INM state and health queries. |
| **current-connection** | None | Prints ID, name, type, interface, state, and IP addresses for the current connection. |
| **stats** | `<wifi\|cellular\|ethernet>` | Prints total RX/TX byte statistics for the selected connection type. |
| **congestion** | None | Prints the current TCP congestion level. |
| **retry-tx-rate** | None | Prints the retry transmit rate. |
| **channel-interference** | None | Prints the Wi-Fi channel interference value. |
| **ip-conflict** | None | Prints the IP-conflict state. |
| **ip-conflict-detect** | None | Prints whether IP-conflict detection is enabled. |
| **tcpdump** | `<start\|stop\|state>` | Starts, stops, or queries the INM tcpdump state. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, INM initialization failure, daemon unavailability,
unsupported runtime query, or command failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-intelligent-network-monitoring-cli`

## AUTHOR
Part of the **Tizen Tools** project.

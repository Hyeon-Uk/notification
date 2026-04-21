# TIZEN-RUNTIME-INFO-CLI(1)

## NAME
**tizen-runtime-info-cli** - Query Tizen runtime information.

## SYNOPSIS
**tizen-runtime-info-cli** *COMMAND* [*OPTIONS*]  
**tizen-runtime-info-cli** **-h** | **--help** | **help**  
**tizen-runtime-info-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-runtime-info-cli** provides command-line access to the public
runtime information APIs from `runtime_info.h`.

The tool supports:
- reading common runtime boolean states
- reading system memory, CPU, and processor frequency information
- querying specific public runtime keys as boolean, integer, or string values
- querying per-process memory and CPU usage for a single PID

Process-specific usage queries follow the privilege rules of
`runtime_info.h` and may require `http://tizen.org/privilege/systemmonitor`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **status** | None | Standard | Prints a summary of common boolean runtime states. |
| **memory** | None | Standard | Prints system memory information from `runtime_info_get_system_memory_info()`. |
| **cpu** | None | Standard | Prints system CPU usage from `runtime_info_get_cpu_usage()`. |
| **processor** | None | Standard | Prints processor count and current/max frequencies. |
| **get-bool** | `--key <KEY>` | Standard | Queries a public key via `runtime_info_get_value_bool()`. |
| **get-int** | `--key <KEY>` | Standard | Queries a public key via `runtime_info_get_value_int()`. |
| **get-string** | `--key <KEY>` | Standard | Queries a public key via `runtime_info_get_value_string()`. |
| **process-memory** | `--pid <PID>` | `systemmonitor` | Prints memory usage for the specified process. |
| **process-cpu** | `--pid <PID>` | `systemmonitor` | Prints CPU usage ticks for the specified process. |

Supported keys: `bluetooth`, `wifi-hotspot`, `bluetooth-tethering`,
`usb-tethering`, `packet-data`, `data-roaming`, `vibration`,
`audio-jack`, `battery-charging`, `tv-out`, `usb-connected`,
`charger-connected`, `auto-rotation`, `gps-status`,
`audio-jack-status`

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or runtime-info API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-runtime-info-cli`

## AUTHOR
Part of the **Tizen Tools** project.

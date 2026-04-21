# TIZEN-UA-CLI(1)

## NAME
**tizen-ua-cli** - Query Tizen User Awareness service, sensor, user, and device metadata.

## SYNOPSIS
**tizen-ua-cli** *COMMAND* [*OPTIONS*]  
**tizen-ua-cli** **-h** | **--help** | **help**  
**tizen-ua-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-ua-cli** provides one-shot access to the User Awareness client ABI
currently shipped by `libua-api` through `ua-api.h`.

The tool initializes the User Awareness framework for each command,
executes a finite query or round-trip operation, prints a structured
result, and then deinitializes the framework before exiting.

Commands are limited to finite service, sensor, user, and device queries
or updates that fit a non-interactive CLI workflow.

## COMMANDS REFERENCE
| Command | Description |
| --- | --- |
| **list-services** | Initialize UA and list registered services returned by `_uam_get_registered_services()`. |
| **get-default-service** | Initialize UA and print the default service returned by `_uam_get_default_service()`. |
| **get-service --name** *NAME* | Initialize UA, scan the registered service list, and print the matching service entry. |
| **sensor-available --sensor** *SENSOR* | Initialize UA, inspect `_uam_get_available_sensors()`, and print availability and ready state for one sensor. |
| **monitor-roundtrip --sensor** *SENSOR* | Initialize UA, request one sensor through `_uam_request_add_sensor()`, inspect the available mask, and then remove it. |
| **service-info --name** *NAME* [**--presence-threshold** *N* **--absence-threshold** *N*] | Print one registered service entry and optionally update its thresholds through `_uam_update_service()`. |
| **user-info --account** *ACCOUNT* | Initialize UA and print user information resolved through `_uam_request_get_user_by_account()` or the default user fallback. |
| **device-info --mac-type** *TYPE* **--mac** *ADDR* **--device-id** *ID* [**--os** *OS*] | Optionally update one device entry through `_uam_request_update_device()` and then print the device resolved by MAC address. |
| **help** | Show help text. |
| **version** | Show version information. |

Supported sensor names: `bt`, `ble`, `wifi`, `motion`, `light`,
`audio`, `wifi-location`, `uwb`

Supported MAC types: `bt`, `ble`, `wifi`, `p2p`, `wifi-location`, `uwb`

Supported OS types: `undefined`, `tizen`, `android`, `ios`

## EXIT STATUS
**0**: Command completed successfully.  
**1**: Invalid arguments or User Awareness API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-ua-cli`

## AUTHOR
Tizen Tools maintainers.

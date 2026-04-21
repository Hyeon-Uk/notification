# TIZEN-LOCATION-MANAGER-CLI(1)

## NAME
**tizen-location-manager-cli** - Query cached Tizen location state and
manage mock location data.

## SYNOPSIS
**tizen-location-manager-cli** *COMMAND* [*OPTIONS*]  
**tizen-location-manager-cli** **-h** | **--help** | **help**  
**tizen-location-manager-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-location-manager-cli** provides command-line access to public
APIs from `locations.h` in `capi-location-manager`.

The tool focuses on synchronous queries that are practical from a shell:
- checking whether a location method is supported or enabled
- reading cached position, velocity, location, and accuracy data
- calculating a geographic distance between two coordinates
- checking, enabling, setting, and clearing mock location data

Commands that depend on a live provider or developer-mode mock support
may print `[WARN]` messages instead of failing when run from a plain
`sdb shell` session on an emulator or headless target.

Supported methods are `GPS`, `WPS`, `HYBRID`, `PASSIVE`, and `FUSED`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **is-supported** | `<METHOD>` | Standard | Prints whether the given method is supported by `location_manager_is_supported_method()`. |
| **is-enabled** | `<METHOD>` | Standard | Prints whether the given method is enabled in system settings. |
| **last-position** | `<METHOD>` | `location` | Prints cached altitude, latitude, longitude, and timestamp. |
| **last-location** | `<METHOD>` | `location` | Prints cached position, speed, direction, accuracy level, and timestamp. |
| **last-velocity** | `<METHOD>` | `location` | Prints cached climb, direction, speed, and timestamp. |
| **last-accuracy** | `<METHOD>` | `location` | Prints cached accuracy level and horizontal/vertical meters. |
| **distance** | `<START_LAT> <START_LON> <END_LAT> <END_LON>` | Standard | Calculates the distance in meters using `location_manager_get_distance()`. |
| **mock-status** | None | Standard | Prints whether mock location is currently enabled. |
| **mock-enable** | `<true\|false>` | `location` and developer mode | Enables or disables mock location globally. |
| **mock-set** | `<LAT> <LON> <ALT> <SPEED> <DIR> <ACC> [--method <METHOD>]` | `location` and developer mode | Sets mock location data for the selected method. Defaults to `GPS`. |
| **clear-mock** | `[--method <METHOD>]` | `location` and developer mode | Clears mock location data for the selected method. Defaults to `GPS`. |

## EXIT STATUS
**0**: Success, or a documented environment fallback reported as
`[WARN]`.  
**1**: Invalid arguments or an unexpected CLI/runtime failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-location-manager-cli`

## AUTHOR
Part of the **Tizen Tools** project.

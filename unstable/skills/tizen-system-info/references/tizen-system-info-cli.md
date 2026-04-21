# TIZEN-SYSTEM-INFO-CLI(1)

## NAME
**tizen-system-info-cli** - Query Tizen platform feature keys and common
feature groups.

## SYNOPSIS
**tizen-system-info-cli** *COMMAND* [*OPTIONS*]  
**tizen-system-info-cli** **-h** | **--help** | **help**  
**tizen-system-info-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-system-info-cli** provides command-line access to the public
System Information APIs from `system_info.h`.

The tool supports:
- querying platform keys as boolean, integer, double, or string values
- querying OEM custom keys as boolean, integer, double, or string values
- checking curated feature groups used by nearby tools and local UTC
  coverage
- validating common STT-related feature support with
  `system_info_get_platform_bool()`

The CLI intentionally focuses on public key-based lookups only.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **get-bool** | `<KEY>` | Gets a boolean platform feature with `system_info_get_platform_bool()`. |
| **get-int** | `<KEY>` | Gets an integer platform feature with `system_info_get_platform_int()`. |
| **get-double** | `<KEY>` | Gets a double platform feature with `system_info_get_platform_double()`. |
| **get-str** | `<KEY>` | Gets a string platform feature with `system_info_get_platform_string()`. |
| **get-custom-bool** | `<KEY>` | Gets a boolean custom feature with `system_info_get_custom_bool()`. |
| **get-custom-int** | `<KEY>` | Gets an integer custom feature with `system_info_get_custom_int()`. |
| **get-custom-double** | `<KEY>` | Gets a double custom feature with `system_info_get_custom_double()`. |
| **get-custom-str** | `<KEY>` | Gets a string custom feature with `system_info_get_custom_string()`. |
| **feature-set** | `--group <stt|connectivity|display>` | Queries a curated group of related feature keys and prints each value. |

Supported feature groups:
- `stt`
- `connectivity`
- `display`

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or System Information API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-system-info-cli`

## AUTHOR
Part of the **Tizen Tools** project.

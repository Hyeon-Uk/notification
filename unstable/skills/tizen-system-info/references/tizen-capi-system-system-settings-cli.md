# TIZEN-CAPI-SYSTEM-SYSTEM-SETTINGS-CLI(1)

## NAME
**tizen-capi-system-system-settings-cli** - Inspect and modify Tizen
system settings keys.

## SYNOPSIS
**tizen-capi-system-system-settings-cli** *COMMAND* [*OPTIONS*]  
**tizen-capi-system-system-settings-cli** **-h** | **--help** | **help**  
**tizen-capi-system-system-settings-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-capi-system-system-settings-cli** provides command-line access
to the native Tizen System Settings APIs from `system_settings.h`.

The tool accepts system setting keys either as numeric IDs such as `3`
or as symbolic names such as `font-size`,
`SYSTEM_SETTINGS_KEY_FONT_SIZE`, and `device-name`.

It supports:
- listing the known system setting keys compiled into the tool
- describing each key's type and access mode
- reading values with automatic type dispatch
- reading or writing specific `int`, `bool`, and `string` keys
- iterating the ringtone string-list key supported by the native API

Write operations use platform APIs and typically require
`http://tizen.org/privilege/systemsettings.admin`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list-keys** | None | Standard | Lists known keys with ID, type, access mode, string-list support, and enum name. |
| **describe-key** | `--key <NAME\|ID>` | Standard | Prints detailed metadata for one key. |
| **get** | `--key <NAME\|ID>` | Standard | Reads a key using its declared type and prints the value with metadata. |
| **get-int** | `--key <NAME\|ID>` | Standard | Reads an integer key and prints the raw integer value. |
| **set-int** | `--key <NAME\|ID> --value <INT>` | Platform `systemsettings.admin` | Sets an integer key. |
| **get-bool** | `--key <NAME\|ID>` | Standard | Reads a boolean key and prints `true` or `false`. |
| **set-bool** | `--key <NAME\|ID> --value <true\|false>` | Platform `systemsettings.admin` | Sets a boolean key. |
| **get-string** | `--key <NAME\|ID>` | Standard | Reads a string key and prints the raw string value. |
| **set-string** | `--key <NAME\|ID> --value <TEXT>` | Platform `systemsettings.admin` | Sets a string key. |
| **add-string** | `--key <NAME\|ID> --value <TEXT>` | Platform `systemsettings.admin` | Appends a string-list entry. Currently only `incoming-call-ringtone` supports this. |
| **delete-string** | `--key <NAME\|ID> --value <TEXT>` | Platform `systemsettings.admin` | Removes a string-list entry. Currently only `incoming-call-ringtone` supports this. |
| **foreach-string** | `--key <NAME\|ID>` | Platform `systemsettings.admin` | Iterates all string-list entries. Currently only `incoming-call-ringtone` supports this. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, unsupported key/type combinations, API
failure, or privilege failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-capi-system-system-settings-cli`

## AUTHOR
Part of the **Tizen Tools** project.

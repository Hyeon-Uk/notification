# TIZEN-PRIVILEGE-INFO-CLI(1)

## NAME
**tizen-privilege-info-cli** - Query public Tizen privilege metadata from a
non-interactive shell.

## SYNOPSIS
**tizen-privilege-info-cli** *COMMAND* [*OPTIONS*]  
**tizen-privilege-info-cli** **-h** | **--help** | **help**  
**tizen-privilege-info-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-privilege-info-cli** exposes the public privilege information APIs from
`privilege_information.h`.

The tool focuses on finite lookup commands that are useful in shell-driven
validation:
- resolving a privilege display name
- resolving a privilege description
- resolving the legacy privacy group display name
- retrieving a locale-aware privilege information list

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **name** | `<privilege-uri> [api-version] [native\|web]` | Prints the privilege display name using `privilege_info_get_display_name()` or `privilege_info_get_display_name_by_pkgtype()`. |
| **desc** | `<privilege-uri> [api-version] [native\|web]` | Prints the privilege description using `privilege_info_get_description()` or `privilege_info_get_description_by_pkgtype()`. |
| **privacy-name** | `<privilege-uri>` | Prints the legacy privacy group display name using `privilege_info_get_privacy_display_name()`. |
| **list** | `[--locale <LOCALE>] --privilege <URI> [--privilege <URI> ...]` | Prints locale-aware privilege name, display name, and description entries from `privilege_info_get_privilege_info_list()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or privilege info API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-privilege-info-cli`

## AUTHOR
Part of the **Tizen Tools** project.

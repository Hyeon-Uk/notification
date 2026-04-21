# TIZEN-BASE-UTILS-CLI(1)

## NAME
**tizen-base-utils-cli** - Query Tizen i18n locale and timezone information.

## SYNOPSIS
**tizen-base-utils-cli** *COMMAND* [*ARGS...*]  
**tizen-base-utils-cli** **-h** | **--help** | **help**  
**tizen-base-utils-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-base-utils-cli** provides synchronous access to a focused subset of
the Tizen `utils_i18n.h` APIs. It is intended for lightweight inspection of
the current locale and timezone state without requiring the `ecore` main loop.

The tool can print:
- the default locale and ISO3 language/country values
- extended locale fields such as language, script, variant, full name, and base name
- the default timezone or a specific timezone ID with region, raw offset, and DST savings

## COMMANDS REFERENCE

| Command | Args | Execution Mode | Feature Description |
| :--- | :--- | :---: | :--- |
| **locale** | none | `Synchronous` | Prints the default locale ID and its ISO3 language/country codes. |
| **locale-details** | none | `Synchronous` | Prints extended locale fields from the default locale using public `i18n_ulocale_*` getters. |
| **timezone** | none | `Synchronous` | Prints the default timezone ID, region, raw offset, and DST savings. |
| **timezone-info** | `[tzid]` | `Synchronous` | Prints timezone details for the specified timezone ID, or for the default timezone when omitted. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or Tizen API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-base-utils-cli`

## AUTHOR
Part of the **Tizen Tools** project.

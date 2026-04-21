# TIZEN-APP-CONTROL-CLI(1)

## NAME
**tizen-app-control-cli** - inspect and round-trip public app-control handles.

## SYNOPSIS
**tizen-app-control-cli** *COMMAND* [*OPTIONS*]  
**tizen-app-control-cli** **-h** | **--help** | **help**  
**tizen-app-control-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-app-control-cli** provides command-line access to the public
Tizen app-control APIs from `app_control.h`.

Each command creates an `app_control_h` handle when needed, performs a
finite setter/getter or clone workflow, prints the observed result, and
destroys the handle before exiting.

The tool focuses on public handle construction, operation preset
inspection, launch resolution, and finite launch-request workflows that
can be validated from a non-interactive shell session.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list-known-operations** | None | Standard | Prints curated public operation presets from `app_control.h`. |
| **list-matched-apps** | `[--operation <PRESET_OR_URI>] [--uri <URI>] [--mime <MIME>] [--category <CATEGORY>] [--app-id <APP_ID>] [--mode <single\|group>] [--extras-json <JSON_OBJECT>]` | Standard | Prints application IDs returned by `app_control_foreach_app_matched()` for implicit/default launch resolution. |
| **launch** | `[--operation <PRESET_OR_URI>] [--uri <URI>] [--mime <MIME>] [--category <CATEGORY>] [--app-id <APP_ID>] [--mode <single\|group>] [--extras-json <JSON_OBJECT>]` | Standard | Sends a finite launch request through `app_control_send_launch_request()` and prints the matched application IDs first. |

`--extras-json` accepts a JSON object whose values are either strings or
string arrays, for example `{"title":"Tizen Tools","tags":["cli","test"]}`.
The schema is strict: the object must not be empty, keys must be
non-empty and unique, arrays must be non-empty, and array items must be
unique strings.

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or an app-control API call failed.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-app-control-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-EFL-UTIL-CLI(1)

## NAME
**tizen-efl-util-cli** - Generate EFL input events from the command line.

## SYNOPSIS
**tizen-efl-util-cli** **input** *SUBCOMMAND* [*ARGS*]  
**tizen-efl-util-cli** **-h** | **--help** | **help**  
**tizen-efl-util-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-efl-util-cli** provides one-shot wrappers over the public EFL util
input generator APIs from `efl_util.h`. The tool is intended for scripted
UI-driving scenarios on devices that expose the input generator privilege and a
Wayland/Ecore runtime.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **input key** | `<key_name>` | Platform `inputgenerator` | Sends a key press and release pair. |
| **input touch** | `<x> <y>` | Platform `inputgenerator` | Sends a single touch tap sequence. |
| **input drag** | `<x1> <y1> <x2> <y2> [steps]` | Platform `inputgenerator` | Sends a drag gesture using `TOUCH_BEGIN`, `TOUCH_UPDATE`, and `TOUCH_END`. |
| **input touch-axis** | `<x> <y> <radius_x> <radius_y> <pressure> <angle> <palm>` | Platform `inputgenerator` | Sends touch events with ellipse and pressure metadata via `efl_util_input_generate_touch_axis()`. |
| **input pointer** | `<button> <x> <y>` | Platform `inputgenerator` | Sends pointer move, down, and up events. |
| **input wheel** | `<vert\|horz> <value>` | Platform `inputgenerator` | Sends a pointer wheel event. |
| **input set-touch-count** | `<max_count>` | Platform `inputgenerator` | Requests a higher maximum touch count from the display server. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, permission failure, missing Wayland/Ecore runtime, or
FFI call failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-efl-util-cli`

## AUTHOR
Part of the **Tizen Tools** project.

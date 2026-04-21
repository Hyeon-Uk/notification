# TIZEN-TZSH-QUICKPANEL-CLI(1)

## NAME
**tizen-tzsh-quickpanel-cli** - Query and control public TZSH quickpanel
client state for an EFL window.

## SYNOPSIS
**tizen-tzsh-quickpanel-cli** *COMMAND* [*OPTIONS*]  
**tizen-tzsh-quickpanel-cli** **-h** | **--help** | **help**  
**tizen-tzsh-quickpanel-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-tzsh-quickpanel-cli** provides command-line access to the public
TZSH quickpanel client APIs from `tzsh.h` and `tzsh_quickpanel.h`.

For each invocation, the tool initializes Elementary, creates a
temporary EFL window, creates a `tzsh_h` context, performs the requested
quickpanel operation, and then releases all native resources.

Only public client-side quickpanel APIs are exposed. Internal quickpanel
service APIs are intentionally not documented or surfaced.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **get-type** | `[--type <system-default\|context-menu\|apps-menu>]` | Prints the resolved quickpanel type for the created client handle. |
| **show** | `[--type <system-default\|context-menu\|apps-menu>]` | Requests the quickpanel service window to show. |
| **hide** | `[--type <system-default\|context-menu\|apps-menu>]` | Requests the quickpanel service window to hide. |
| **get-visible** | `[--type <system-default\|context-menu\|apps-menu>]` | Prints whether the quickpanel is shown, hidden, or unknown. |
| **set-scrollable-state** | `--state <set\|unset\|retain> [--type <system-default\|context-menu\|apps-menu>]` | Sets the per-window scrollable state used by the quickpanel service. |
| **get-scrollable-state** | `[--type <system-default\|context-menu\|apps-menu>]` | Prints the current scrollable state for the window. |
| **get-orientation** | `[--type <system-default\|context-menu\|apps-menu>]` | Prints the quickpanel orientation state. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, window setup failure, or quickpanel API
failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-tzsh-quickpanel-cli`

## AUTHOR
Part of the **Tizen Tools** project.

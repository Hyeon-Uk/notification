# TIZEN-CAPI-UI-INPUTMETHOD-MANAGER-CLI(1)

## NAME
**tizen-capi-ui-inputmethod-manager-cli** - Inspect and manage the
system input method manager state.

## SYNOPSIS
**tizen-capi-ui-inputmethod-manager-cli** *COMMAND* [*OPTIONS*]  
**tizen-capi-ui-inputmethod-manager-cli** **-h** | **--help** | **help**  
**tizen-capi-ui-inputmethod-manager-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-capi-ui-inputmethod-manager-cli** provides command-line access
to the public Tizen Input Method Manager APIs from
`inputmethod_manager.h`.

The tool supports:
- printing a one-shot summary of the current active IME and enabled IME
  count
- showing the IME list and IME selector UI
- checking whether a specific IME app ID is enabled
- printing the active IME app ID
- printing the number of enabled IMEs
- requesting IME prelaunch

Most commands require the platform privilege
`http://tizen.org/privilege/imemanager`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **status** | None | Platform `imemanager` | Prints `active_ime`, `active_ime_enabled`, and `enabled_ime_count` in one snapshot. |
| **show-list** | None | Platform `imemanager` | Requests the system IME list UI. |
| **show-selector** | None | Platform `imemanager` | Requests the system IME selector UI. |
| **is-enabled** | `--app-id <ID>` | Platform `imemanager` | Prints whether the given IME application ID is enabled. |
| **get-active** | None | Platform `imemanager` | Prints the currently active IME application ID, or `none` when unavailable. |
| **get-count** | None | Platform `imemanager` | Prints the number of enabled IMEs. |
| **prelaunch** | None | Platform `imemanager` | Requests prelaunch of the current IME implementation. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, API failure, privilege failure, or unavailable
IME manager state.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-capi-ui-inputmethod-manager-cli`

## AUTHOR
Part of the **Tizen Tools** project.

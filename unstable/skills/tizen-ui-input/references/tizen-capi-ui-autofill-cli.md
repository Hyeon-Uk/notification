# TIZEN-CAPI-UI-AUTOFILL-CLI(1)

## NAME
**tizen-capi-ui-autofill-cli** - Inspect and manage the current Tizen
Autofill service.

## SYNOPSIS
**tizen-capi-ui-autofill-cli** *COMMAND* [*OPTIONS*]  
**tizen-capi-ui-autofill-cli** **-h** | **--help** | **help**  
**tizen-capi-ui-autofill-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-capi-ui-autofill-cli** provides command-line access to the
native Tizen Autofill Manager APIs from `autofill_manager.h`.

The tool supports:
- checking whether the client can connect to the Autofill manager daemon
- printing the currently selected Autofill service app ID
- listing available Autofill service app IDs
- printing a one-shot summary of connection state, current service, and
  service count
- assigning a new Autofill service app ID

Most commands require the platform privilege
`http://tizen.org/privilege/autofillmanager`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **status** | None | Platform `autofillmanager` | Connects to the Autofill manager daemon and prints the resulting connection status. |
| **manager-info** | None | Platform `autofillmanager` | Prints the connection state, current service app ID, total service count, and each available service. |
| **manager-get** | None | Platform `autofillmanager` | Prints the currently selected Autofill service app ID. |
| **manager-list** | None | Platform `autofillmanager` | Lists all available Autofill service app IDs. |
| **manager-set** | `--app-id <ID>` | Platform `autofillmanager` | Sets the current Autofill service app ID. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, connection failure, RPC failure, privilege
failure, or unavailable service state.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-capi-ui-autofill-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-CAPI-UI-INPUTMETHOD-CLI(1)

## NAME
**tizen-capi-ui-inputmethod-cli** - Inspect and control Tizen remote
input sessions for the focused text field.

## SYNOPSIS
**tizen-capi-ui-inputmethod-cli** *COMMAND* [*OPTIONS*]  
**tizen-capi-ui-inputmethod-cli** **-h** | **--help** | **help**  
**tizen-capi-ui-inputmethod-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-capi-ui-inputmethod-cli** provides command-line access to the
public `remote_input.h` APIs from the Tizen Input Method stack.

The tool supports:
- checking whether a remote input session can connect
- printing current remote input metadata for the focused field
- sending remote key events
- committing text and updating preedit text
- deleting surrounding text
- moving the cursor in the focused field

Most commands require the platform privilege
`http://tizen.org/privilege/internal/default/platform`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **status** | None | Platform internal default platform | Connects to the remote input service and prints `connected` or `rejected`. |
| **metadata** | None | Platform internal default platform | Prints current metadata values for the focused field, including layout and return-key attributes. |
| **send-key** | `--key <NAME>` | Platform internal default platform | Sends a supported remote key event to the focused field. |
| **commit-string** | `--text <TEXT>` | Platform internal default platform | Commits UTF-8 text to the focused field. |
| **update-preedit** | `--text <TEXT>` [`--cursor-pos <POS>`] | Platform internal default platform | Updates preedit text and optionally sets its cursor position. |
| **delete-surrounding** | `--offset <OFFSET>` `--len <LEN>` | Platform internal default platform | Deletes surrounding text relative to the cursor position. |
| **set-cursor** | `--pos <POS>` | Platform internal default platform | Requests a new cursor position in the focused field. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, connection failure, privilege failure, or
remote input operation failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-capi-ui-inputmethod-cli`

## AUTHOR
Part of the **Tizen Tools** project.

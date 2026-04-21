# TIZEN-NATIVE-COMMON-CLI(1)

## NAME
**tizen-native-common-cli** - Inspect core Tizen common error values and
thread-local error state.

## SYNOPSIS
**tizen-native-common-cli** *COMMAND* [*OPTIONS*]  
**tizen-native-common-cli** **-h** | **--help** | **help**  
**tizen-native-common-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-native-common-cli** provides command-line access to the common
error helpers from `tizen_error.h`.

The tool is intended for quick inspection of:
- the current thread-local last-result value
- system error messages for integer error codes
- public common Tizen error constants
- whether a code is classified as success, platform error, or module error

Because the last-result value is thread-local, a value set in one CLI
invocation is not expected to persist into the next process.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **get-last-result** | None | Prints the current thread-local error code returned by `get_last_result()`. |
| **set-last-result** | `--err <CODE>` | Sets the current thread-local error code using `set_last_result()`. |
| **get-error-message** | `--err <CODE>` | Prints the message string returned by `get_error_message()`. |
| **explain** | `--err <CODE>` | Prints the error message and the common `tizen_error.h` classification for the given code. |
| **list-common-errors** | None | Prints a compact table of representative public common Tizen error constants. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or a failure to resolve the requested error
message.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-native-common-cli`

## AUTHOR
Part of the **Tizen Tools** project.

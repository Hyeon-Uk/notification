# TIZEN-VOICE-CONTROL-CLI(1)

## NAME
**tizen-voice-control-cli** - query public voice control client state and
basic dialogue helpers through `voice_control.h`.

## SYNOPSIS
**tizen-voice-control-cli** *COMMAND* [*OPTIONS*]  
**tizen-voice-control-cli** **-h** | **--help** | **help**  
**tizen-voice-control-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-voice-control-cli** provides command-line access to the public
voice control client APIs from `voice_control.h` and
`voice_control_command.h`.

Each invocation initializes the voice control client, runs one finite
operation, prints the observed result, and deinitializes the client
before exiting. Commands that require the service to be ready perform a
prepare/unprepare roundtrip internally.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **languages** | none | Lists languages reported by `vc_foreach_supported_languages()`. |
| **current-language** | none | Prints the current voice control language from `vc_get_current_language()`. |
| **states** | none | Prints the client state and, when available, the service state. |
| **prepare-roundtrip** | none | Prepares the service, prints language and state information, then unprepares it. |
| **system-command-count** | none | Prepares the service, gets the system command list, and prints its command count. |
| **set-invocation-name** | `<name>` | Prepares the service and sets the invocation name with `vc_set_invocation_name()`. |
| **request-dialog** | `<disp_text> <utt_text> <auto_start>` | Prepares the service and requests a dialogue using `vc_request_dialog()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or a voice control API call failed.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-voice-control-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-MULTI-ASSISTANT-CLI(1)

## NAME
**tizen-multi-assistant-cli** - Query the Tizen multi-assistant client
and settings APIs.

## SYNOPSIS
**tizen-multi-assistant-cli** *COMMAND* [*OPTIONS*]  
**tizen-multi-assistant-cli** **-h** | **--help** | **help**  
**tizen-multi-assistant-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-multi-assistant-cli** provides command-line access to the public
Multi Assistant client APIs from `multi_assistant.h` and
`multi_assistant_settings.h`.

The tool focuses on one-shot inspection commands that can be validated
on real devices and emulators:
- initializing and preparing the client lifecycle
- querying the current client state
- querying current language and recording audio format
- querying multiple-mode and current/default assistant settings
- checking whether a specific assistant app ID is enabled

On devices where the multi-assistant runtime is unavailable, not
supported, or blocked by permissions, the CLI reports a documented
`[WARN]` result instead of treating the environment as a hard failure.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **init** | None | `http://tizen.org/privilege/recorder` | Initializes the multi-assistant client and immediately deinitializes it after the probe. |
| **prepare** | None | `http://tizen.org/privilege/recorder` | Initializes and prepares the client, printing a documented warning if the runtime is unavailable. |
| **state** | None | `http://tizen.org/privilege/recorder` | Prints the current client state returned by `ma_get_state()`. |
| **current-language** | None | `http://tizen.org/privilege/recorder` | Prints the current language returned by `ma_get_current_language()`. |
| **recording-format** | None | `http://tizen.org/privilege/recorder` | Prints the recording sample rate, channel count, and audio type returned by `ma_get_recording_audio_format()`. |
| **multiple-mode** | None | Standard | Prints whether multiple-assistant mode is enabled. |
| **current-assistant** | None | Standard | Prints the currently active voice assistant app ID in single-assistant mode. |
| **default-assistant** | None | Standard | Prints the default voice assistant app ID in multiple-assistant mode. |
| **assistant-enabled** | `--app-id <APP_ID>` | Standard | Prints whether the specified voice assistant app ID is enabled. |

## EXIT STATUS
**0**: Success, or a documented runtime fallback reported with `[WARN]`.  
**1**: Invalid arguments or an unrecoverable API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-multi-assistant-cli`

## AUTHOR
Part of the **Tizen Tools** project.

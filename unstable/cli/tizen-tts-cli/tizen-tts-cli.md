# TIZEN-TTS-CLI(1)

## NAME
**tizen-tts-cli** - Query and exercise the public Tizen text-to-speech
client API.

## SYNOPSIS
**tizen-tts-cli** *COMMAND* [*OPTIONS*]  
**tizen-tts-cli** **-h** | **--help** | **help**  
**tizen-tts-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-tts-cli** provides command-line access to the public client-side
TTS APIs from `tts.h`.

The tool supports:
- reading the current TTS mode and client state
- reading the current TTS service state after synchronous preparation
- reading the current speech speed range
- reading the current default voice
- checking whether screen reader mode is enabled
- speaking one queued utterance and waiting for completion

Some runtime behavior depends on device policy, the installed TTS
engine, audio routing, and whether the
`http://tizen.org/feature/speech.synthesis` feature is enabled.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **get-mode** | None | Standard | Calls `tts_get_mode()` and prints the current mode. |
| **get-state** | None | Standard | Calls `tts_get_state()` and prints the current client state. |
| **get-service-state** | None | Standard | Calls `tts_prepare_sync()` and `tts_get_service_state()` to print the current service state. |
| **get-speed-range** | None | Standard | Calls `tts_get_speed_range()` and prints min, normal, and max speed values. |
| **get-default-voice** | None | Standard | Calls `tts_prepare_sync()` and `tts_get_default_voice()` to print the default language and voice type. |
| **check-screen-reader** | None | Standard | Calls `tts_check_screen_reader_on()` and prints whether screen reader mode is enabled. |
| **synthesize** | `<TEXT>` | Standard | Calls `tts_prepare_sync()`, `tts_add_text()`, and `tts_play()` to speak one utterance and wait for completion. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or TTS API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-tts-cli`

## AUTHOR
Part of the **Tizen Tools** project.

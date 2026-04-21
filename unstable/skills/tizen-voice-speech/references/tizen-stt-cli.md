# TIZEN-STT-CLI(1)

## NAME
**tizen-stt-cli** - Query Tizen Speech-to-Text feature support and basic
engine information.

## SYNOPSIS
**tizen-stt-cli** *COMMAND* [*OPTIONS*]  
**tizen-stt-cli** **-h** | **--help** | **help**  
**tizen-stt-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-stt-cli** provides command-line access to the public Tizen
Speech-to-Text APIs from `stt.h` and uses `system_info_get_platform_bool()`
to verify related device feature support.

The tool focuses on finite query and configuration checks:
- checking whether microphone and speech-recognition features are supported
- creating an STT handle and printing its initial state
- listing supported engines and languages
- retrieving the current engine and default language
- checking support for common public recognition types
- getting or setting the current audio input ID on a temporary handle

Most commands require the public
`http://tizen.org/privilege/recorder` privilege. When the platform,
permission, or engine support is unavailable at runtime, the CLI prints
`[WARN]` instead of failing so capability gaps remain visible without
turning device smoke tests into false failures.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **features** | None | Standard | Checks `http://tizen.org/feature/microphone` and `http://tizen.org/feature/speech.recognition` through `system_info_get_platform_bool()`. |
| **state** | None | `http://tizen.org/privilege/recorder` | Creates an STT handle and prints the current `stt_state_e` value. |
| **engine** | None | `http://tizen.org/privilege/recorder` | Prints the current STT engine ID from `stt_get_engine()`. |
| **list-engines** | None | `http://tizen.org/privilege/recorder` | Lists supported engines via `stt_foreach_supported_engines()`. |
| **default-language** | None | `http://tizen.org/privilege/recorder` | Prints the current default language from `stt_get_default_language()`. |
| **list-languages** | None | `http://tizen.org/privilege/recorder` | Lists languages supported by the current engine via `stt_foreach_supported_languages()`. |
| **recognition-types** | None | `http://tizen.org/privilege/recorder` | Reports support status for `FREE`, `FREE.PARTIAL`, `SEARCH`, `WEB_SEARCH`, and `MAP` recognition types. |
| **audio-id** | None | `http://tizen.org/privilege/recorder` | Prints the current audio input ID from `stt_get_audio_id()`. |
| **set-audio-id** | `--value <bluetooth|wifi|none|AUDIO_ID>` | `http://tizen.org/privilege/recorder` | Sets an audio input ID with `stt_set_audio_id()` and reads it back on the same handle. |

## EXIT STATUS
**0**: Success, or a documented runtime fallback reported with `[WARN]`.  
**1**: Invalid arguments or an unrecoverable feature-query failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-stt-cli`

## AUTHOR
Part of the **Tizen Tools** project.

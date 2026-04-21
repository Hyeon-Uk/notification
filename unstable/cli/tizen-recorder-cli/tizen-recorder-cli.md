# TIZEN-RECORDER-CLI(1)

## NAME
**tizen-recorder-cli** - Query and exercise Tizen recorder APIs.

## SYNOPSIS
**tizen-recorder-cli** *COMMAND* [*OPTIONS*]  
**tizen-recorder-cli** **-h** | **--help** | **help**  
**tizen-recorder-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-recorder-cli** provides command-line access to the public native
Tizen recorder APIs from `recorder.h`.

The tool supports:
- querying the initial recorder state
- listing supported audio encoders and file formats
- configuring a recorder and reading the applied values back
- running finite record and cancel lifecycle demos

Recording commands typically require the public
`http://tizen.org/privilege/recorder` privilege and a writable target
path.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **state** | None | Standard | Creates an audio recorder and prints its current lifecycle state. |
| **supported-codecs** | None | Standard | Lists audio encoders returned by `recorder_foreach_supported_audio_encoder()`. |
| **supported-formats** | None | Standard | Lists file formats returned by `recorder_foreach_supported_file_format()`. |
| **setup** | `<codec> <format> <file>` | Recorder | Configures a recorder and confirms the applied codec, format, filename, and state. |
| **info** | `<codec> <format> <file>` | Recorder | Creates a recorder with the provided values and prints the queried recorder configuration. |
| **record** | `<codec> <format> <file>` | Recorder and storage | Prepares, starts, and commits a finite recording session. |
| **cancel-demo** | `<codec> <format> <file>` | Recorder and storage | Prepares, starts, pauses, and cancels a finite recording session. |

Supported codecs: `amr`, `aac`, `vorbis`, `pcm`, `mp3`  
Supported formats: `3gp`, `mp4`, `amr`, `wav`, `ogg`, `m2ts`

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or recorder API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-recorder-cli`

## AUTHOR
Part of the **Tizen Tools** project.

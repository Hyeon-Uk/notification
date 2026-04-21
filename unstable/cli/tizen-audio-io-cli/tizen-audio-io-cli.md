# TIZEN-AUDIO-IO-CLI(1)

## NAME
**tizen-audio-io-cli** - Inspect and exercise Tizen native audio input and output.

## SYNOPSIS
**tizen-audio-io-cli** *COMMAND* [*ARGS...*]  
**tizen-audio-io-cli** **-h** | **--help** | **help**  
**tizen-audio-io-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-audio-io-cli** provides direct command-line access to the native
Tizen Audio I/O APIs in `audio_io.h`.

The tool supports:
- printing default runtime behavior and version information
- querying input and output stream properties from real native handles
- reading and updating input volume
- continuous asynchronous record and playback using the `ecore` main loop
- finite synchronous input reads and output writes for automation-friendly checks

Some input commands may require recorder privilege or available audio
hardware on the target device.

## COMMANDS REFERENCE

| Command | Args | Mode | Description |
| :--- | :--- | :---: | :--- |
| **info** | None | `sync` | Prints the tool defaults and the async/sync execution model. |
| **input-info** | `[sample_rate] [channels]` | `sync` | Creates an input handle and prints buffer size, sample rate, channel, sample type, and current input volume. |
| **output-info** | `[sample_rate] [channels]` | `sync` | Creates an output handle and prints buffer size, sample rate, channel, sample type, and sound type. |
| **get-input-volume** | `[sample_rate] [channels]` | `sync` | Reads the current input volume from an input handle. |
| **set-input-volume** | `<value> [sample_rate] [channels]` | `sync` | Sets the input volume. `<value>` must be in the range `0.0..1.0`. |
| **record** | `[sample_rate] [channels]` | `async` | Starts continuous microphone capture and writes PCM to `stdout` until interrupted. |
| **play** | `[sample_rate] [channels]` | `async` | Starts continuous playback and consumes PCM from `stdin` until EOF or interruption. |
| **record-sync** | `[sample_rate] [channels] [buffers]` | `sync` | Reads a finite number of input buffers and writes PCM to `stdout`. |
| **play-silence** | `[sample_rate] [channels] [buffers]` | `sync` | Writes a finite number of silent PCM buffers to the output device. |

## DEFAULTS
- Sample Rate: `44100`
- Channels: `2`
- Sample Type: `AUDIO_SAMPLE_TYPE_S16_LE`
- Sync buffer count for finite commands: `1`

## VALIDATION
- Sample rate must be between `8000` and `192000`
- Channel count must be between `1` and `16`
- Buffer count must be greater than `0`
- Volume must be between `0.0` and `1.0`

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, Tizen API failure, privilege failure, or device/audio
resource failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-audio-io-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# axfer - ALSA audio transfer tool

## NAME

**axfer** - Transfer audio data to/from ALSA PCM devices.

## SYNOPSIS

```bash
axfer transfer <capture|playback> [options] [file]
axfer list <capture|playback>
axfer version
axfer help
```

## DESCRIPTION

**axfer** is a modern command-line sound transfer tool intended to be the successor to `aplay` and `arecord`. It handles audio frame transfers strictly, resolving limitations of the older tools particularly around large file formats and latency.

## COMMANDS

| Command | Description |
|---------|-------------|
| `transfer` | Execute audio playback or recording depending on direction. |
| `list` | List available PCM nodes/interfaces for the given direction. |
| `version` | Show axfer version. |
| `help` | Print help menu. |

## DIRECTIONS

| Direction | Description |
|-----------|-------------|
| `capture` | Record audio from device. |
| `playback` | Play audio to device. |

## OPTIONS

| Option | Description |
|--------|-------------|
| `-D` | Device name (e.g., `default`, `hw:0,0`). |
| `-f` | Sample format (e.g., `S16_LE`, `cd`, `dat`). |
| `-c` | Number of channels. |
| `-r` | Sample rate in Hz. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The transfer or listing completed successfully. |
| `1` | Error. Device busy, hardware mapping error, or incompatible PCM parameters. |

## EXAMPLES

### List playback devices

```bash
$ axfer list playback
```

### List capture devices

```bash
$ axfer list capture
```

### Play audio file

```bash
$ axfer transfer playback -D default file.wav
```

### Record audio

```bash
$ axfer transfer capture -D default -f cd file.wav
```

### Record with specific format

```bash
$ axfer transfer capture -D hw:0,0 -f S16_LE -c 2 -r 48000 file.wav
```

### Show version

```bash
$ axfer version
```

## NOTES

- Directly utilizes standard `asound.conf` mappings (`hw`, `plughw`, `default`).
- Abstracts recording and playback behind the `transfer` subcommand.
- Avoids the need for two separate binaries like `aplay`/`arecord`.

## SEE ALSO

**aplay**(1), **arecord**(1), **amixer**(1)

## PACKAGE

`alsa-utils`
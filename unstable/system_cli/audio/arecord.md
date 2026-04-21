# arecord - ALSA command-line sound recorder

## NAME

**arecord** - Record audio from ALSA soundcard drivers.

## SYNOPSIS

```bash
arecord [options] [file]
```

## DESCRIPTION

**arecord** is a command-line sound recorder for ALSA soundcard drivers. It reads raw streaming audio buffers from ALSA and saves them to a file or standard output, supporting format encoding like WAV. It shares its codebase with `aplay`.

## OPTIONS

### Device Options

| Option | Description |
|--------|-------------|
| `-l`, `--list-devices` | List all soundcards and digital audio devices. |
| `-L`, `--list-pcms` | List device names. |
| `-D`, `--device` | Select PCM interface by name. |

### Audio Format Options

| Option | Description |
|--------|-------------|
| `-t`, `--file-type` | File type (voc, wav, raw, or au). Default: `wav` |
| `-c`, `--channels` | Number of channels. Default: `1` |
| `-f`, `--format` | Sample format (e.g., S16_LE). |
| `-r`, `--rate` | Sample rate in Hz. Default: `8000` |

### Recording Options

| Option | Description |
|--------|-------------|
| `-d`, `--duration` | Stop recording after # seconds. |
| `-V`, `--vumeter` | Enable VU meter (mono/stereo). |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The recording completed successfully or was gracefully halted. |
| `1` | Error. Device unavailable, format unsupported, or bad syntax. |

## EXAMPLES

### List PCM devices

```bash
$ arecord -L
```

### Record 5 seconds of audio

```bash
$ arecord -d 5 test.wav
```

### Record CD-quality stereo audio

```bash
$ arecord -D hw:1,0 -f cd test.wav
```

### Record with specific format

```bash
$ arecord -D hw:0,0 -f S16_LE -c 2 -r 44100 output.wav
```

### Record to stdout

```bash
$ arecord -t raw -f S16_LE -c 2 -r 48000 | aplay
```

### Record with VU meter

```bash
$ arecord -V stereo -d 10 test.wav
```

## NOTES

- Can be affected by `ALSA_CONFIG_PATH` or standard ALSA configurations like `/etc/asound.conf`.
- Outputs raw/wav binary stream to file or stdout.
- Recording parameters are emitted to stderr.

## SEE ALSO

**aplay**(1), **amixer**(1), **arecordmidi**(1)

## PACKAGE

`alsa-utils`
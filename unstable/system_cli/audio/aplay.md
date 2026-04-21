# aplay - ALSA command-line sound player

## NAME

**aplay** - Play audio files through ALSA soundcard drivers.

## SYNOPSIS

```bash
aplay [options] [file]...
```

## DESCRIPTION

**aplay** is a command-line sound player for ALSA soundcard drivers. It understands raw, wav, voc, and au formats. It reads streaming audio from stdin or designated files and feeds it to the ALSA framework.

## OPTIONS

### Device Options

| Option | Description |
|--------|-------------|
| `-l`, `--list-devices` | List all soundcards and digital audio devices. |
| `-L`, `--list-pcms` | List device names. |
| `-D`, `--device` | Select PCM by name. |

### Audio Format Options

| Option | Description |
|--------|-------------|
| `-t`, `--file-type` | File type (voc, wav, raw, or au). Default: `auto` |
| `-c`, `--channels` | Number of channels. |
| `-f`, `--format` | Sample format (e.g., S16_LE). |
| `-r`, `--rate` | Sample rate in Hz (e.g., 44100). |

### Playback Options

| Option | Description |
|--------|-------------|
| `-d`, `--duration` | Interrupt playback after # seconds. |
| `-q`, `--quiet` | Quiet mode. |
| `-V`, `--vumeter` | Enable VU meter (mono/stereo). |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The file finished playing or early termination requested successfully. |
| `1` | Error. Invalid file format, device busy, or ALSA configuration error. |

## EXAMPLES

### List PCM devices

```bash
$ aplay -L
```

### List all soundcards

```bash
$ aplay -l
```

### Play a WAV file

```bash
$ aplay test.wav
```

### Play on specific device

```bash
$ aplay -D hw:0,0 test.wav
```

### Play raw audio with format specification

```bash
$ aplay -D hw:0,0 -f S16_LE -c 2 -r 44100 raw_audio.bin
```

### Play from stdin

```bash
$ cat audio.raw | aplay -f S16_LE -c 2 -r 48000
```

### Play for 10 seconds

```bash
$ aplay -d 10 test.wav
```

## NOTES

- Can be affected by `ALSA_CONFIG_PATH` or standard ALSA routing in `/etc/asound.conf`.
- Output displays playback settings: `Playing WAVE 'test.wav' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo`.

## SEE ALSO

**arecord**(1), **amixer**(1), **aplaymidi**(1)

## PACKAGE

`alsa-utils`
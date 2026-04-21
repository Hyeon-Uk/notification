# speaker-test - ALSA speaker test tone generator

## NAME

**speaker-test** - Generate test tones for speaker testing.

## SYNOPSIS

```bash
speaker-test [options]...
```

## DESCRIPTION

**speaker-test** is a command-line speaker test tone generator for ALSA. It is typically used to test audio configurations by emitting pink noise, sine waves, or WAV files specifically routed to individual surround/mono/stereo channels mapping to specific physical speakers (e.g., Front Left, Center, Rear Right).

## OPTIONS

### Device Options

| Option | Description |
|--------|-------------|
| `-D`, `--device` | Playback device. Default: `default` |
| `-c`, `--channels` | Count of channels in stream. Default: `1` |
| `-r`, `--rate` | Stream rate in Hz. Default: `48000` |
| `-F`, `--format` | Sample format. Default: `S16_LE` |

### Test Options

| Option | Description |
|--------|-------------|
| `-t`, `--test` | Test mode: `pink`, `sine`, `st2095`, `wav`. Default: `pink` |
| `-f`, `--frequency` | Sine wave frequency in Hz. Default: `440` |
| `-s`, `--speaker` | Single speaker test (1=Left, 2=Right, etc.). |
| `-l`, `--nloops` | Number of loops to test. Default: `0` (infinite) |
| `-S`, `--scale` | Scale of generated test tones in percent. Default: `80` |
| `-X`, `--force-frequency` | Force frequencies outside the 30-8000Hz range. |

### WAV File Options

| Option | Description |
|--------|-------------|
| `-w`, `--wavfile` | Use the given WAV file as a test sound. |
| `-W`, `--wavdir` | Specify the directory containing WAV files. |

### Buffer Options

| Option | Description |
|--------|-------------|
| `-b`, `--buffer` | Ring buffer size in microseconds. |
| `-p`, `--period` | Period size in microseconds. |
| `-P`, `--nperiods` | Number of periods. |
| `-m`, `--chmap` | Specify the channel map to override. |

### Help

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Exited via manual cancellation (CTRL+C) or loop conclusion. |
| `1` | Error. Playback error, bad device syntax, or invalid arguments. |

## EXAMPLES

### Standard stereo test

```bash
$ speaker-test -c 2
```

### 5.1 surround test

```bash
$ speaker-test -D surround51 -c 6
```

### 7.1 surround test

```bash
$ speaker-test -D surround71 -c 8
```

### Sine wave test at specific frequency

```bash
$ speaker-test -t sine -f 1000 -c 2
```

### Test single speaker

```bash
$ speaker-test -s 1 -c 2
```

### Test with WAV file

```bash
$ speaker-test -t wav -w test.wav -c 2
```

### Limited number of loops

```bash
$ speaker-test -c 2 -l 3
```

## NOTES

- Uses `~/.asoundrc` or system `/etc/asound.conf` to target exact speaker routing (`surround51`, `surround71`, etc.).
- Emits alternating text strings tracking which speaker index is currently producing sound to verify spatial mapping accuracy.

## SEE ALSO

**aplay**(1), **amixer**(1), **alsactl**(1)

## PACKAGE

`alsa-utils`
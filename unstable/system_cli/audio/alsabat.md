# alsabat - ALSA Basic Audio Tester

## NAME

**alsabat** - Test audio play and capture functionality on ALSA devices.

## SYNOPSIS

```bash
alsabat [options]...
```

## DESCRIPTION

**alsabat** is the ALSA Basic Audio Tester utility. It tests audio play and capture by generating a signal, playing it, capturing the return, and analyzing the frequency and noise levels (SNR) to determine audio pipeline health.

## OPTIONS

### Device Options

| Option | Description |
|--------|-------------|
| `-D` | PCM device for both playback and capture. |
| `-P` | PCM device for playback. |
| `-C` | PCM device for capture. |

### Audio Format Options

| Option | Description |
|--------|-------------|
| `-f` | Sample format (U8, S16_LE, cd, dat, etc). |
| `-c` | Number of channels. |
| `-r` | Sampling rate. |
| `-n` | Frames to playback or capture. |
| `-B` | Buffer size in frames. |
| `-E` | Period size in frames. |

### Test Configuration Options

| Option | Description |
|--------|-------------|
| `-F` | Target frequency. |
| `-k` | Parameter for frequency detecting threshold. |
| `-p` | Total number of periods to play/capture. |
| `--snr-db` | Noise detect threshold in SNR(dB). |
| `--snr-pc` | Noise detect threshold in noise percentage(%). |

### File Options

| Option | Description |
|--------|-------------|
| `--log` | File for redirecting stdout and stderr. |
| `--file` | File for playback. |
| `--saveplay` | File to save playback content for debug. |
| `--readcapture` | File with previously captured content for analysis. |

### Mode Options

| Option | Description |
|--------|-------------|
| `--local` | Internal loop to bypass PCM hardware devices. |
| `--standalone` | Standalone mode to bypass analysis. |
| `--roundtriplatency` | Round trip latency mode. |

### Help

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help text. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The test passed successfully. |
| `1` | Error. Invalid arguments, device error, or analysis failed. |

## EXAMPLES

### Basic audio test on default device

```bash
$ alsabat -D default
```

### Test with specific format and rate

```bash
$ alsabat -D hw:0 -c 2 -r 48000 -f S16_LE
```

### Save playback content for debugging

```bash
$ alsabat -D hw:0 -c 2 -r 48000 --saveplay=/tmp/out.wav
```

### Standalone mode with file input

```bash
$ alsabat --standalone --file=/tmp/test.wav
```

## NOTES

- Uses internal ALSA configuration (`~/.asoundrc` or `/etc/asound.conf`).
- Outputs test parameters, results, and PASS/FAIL status based on SNR threshold.

## SEE ALSO

**aplay**(1), **arecord**(1), **amixer**(1)

## PACKAGE

`alsa-utils`
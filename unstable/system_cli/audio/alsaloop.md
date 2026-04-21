# alsaloop - ALSA PCM loopback utility

## NAME

**alsaloop** - Route PCM audio from a capture device to a playback device.

## SYNOPSIS

```bash
alsaloop [options]...
```

## DESCRIPTION

**alsaloop** is an ALSA PCM loopback utility. It allows routing PCM audio from a capture device directly to a playback device, effectively looping the audio. It can run in the foreground or as a daemon.

## OPTIONS

### Device Options

| Option | Description |
|--------|-------------|
| `-P`, `--pdevice` | Playback device. |
| `-C`, `--cdevice` | Capture device. |

### Audio Format Options

| Option | Description |
|--------|-------------|
| `-f`, `--format` | Sample format (e.g., S16_LE). |
| `-c`, `--channels` | Number of channels. |
| `-r`, `--rate` | Sampling rate. |

### Latency Options

| Option | Description |
|--------|-------------|
| `-l`, `--latency` | Requested latency in frames. |
| `-t`, `--tlatency` | Requested latency in microseconds. |

### Mode Options

| Option | Description |
|--------|-------------|
| `-d`, `--daemonize` | Daemonize the main process and use syslog for errors. |
| `-z`, `--syslog` | Use syslog for errors. |
| `-g`, `--config` | Configuration file (one line = one job specified). |

### Help

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Ran successfully or daemonized successfully. |
| `1` | Error. Device not found or configuration error. |

## EXAMPLES

### Basic loopback from capture to playback device

```bash
$ alsaloop -C hw:1,0 -P hw:0,0
```

### Low latency background loopback

```bash
$ alsaloop -C hw:1,0 -P hw:0,0 -t 1000 --daemonize
```

### Loopback with specific format and rate

```bash
$ alsaloop -C hw:1,0 -P hw:0,0 -f S16_LE -c 2 -r 48000
```

### Run as daemon with syslog

```bash
$ alsaloop -C hw:1,0 -P hw:0,0 -d -z
```

## NOTES

- Uses standard ALSA library configuration.
- Can read its own configuration file specified by `-g`.
- If not daemonized, outputs continuous connection information and XRUN logs to stdout/stderr.
- Many advanced tuning flags (resample, buffers, sync modes) are available; see `alsaloop --help`.

## SEE ALSO

**aplay**(1), **arecord**(1), **alsactl**(1)

## PACKAGE

`alsa-utils`
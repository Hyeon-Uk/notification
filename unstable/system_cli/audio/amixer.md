# amixer - ALSA command-line mixer

## NAME

**amixer** - Control and query ALSA soundcard mixer settings.

## SYNOPSIS

```bash
amixer [options] [command]
```

## DESCRIPTION

**amixer** is a command-line mixer for ALSA soundcard drivers. It controls and queries audio properties like volume, mute/unmute states, and routing toggles via simple or advanced mixer controls.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-c`, `--card` | Select the card number. |
| `-D`, `--device` | Select the device. Default: `default` |
| `-n`, `--nocheck` | Do not perform range checking. |
| `-q`, `--quiet` | Be quiet. |
| `-i`, `--inactive` | Show also inactive controls. |
| `-s`, `--stdin` | Read and execute commands from stdin. |

## COMMANDS

### Simple Controls

| Command | Description |
|---------|-------------|
| `scontrols` | Show all mixer simple controls. |
| `scontents` | Show contents of all mixer simple controls (default). |
| `sset sID P` | Set contents for one mixer simple control. |
| `sget sID` | Get contents for one mixer simple control. |

### Advanced Controls

| Command | Description |
|---------|-------------|
| `controls` | Show all mixer advanced controls. |
| `contents` | Show contents of all mixer advanced controls. |
| `cset cID P` | Set contents for one mixer advanced control. |
| `cget cID` | Get contents for one mixer advanced control. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The operation completed successfully. |
| `1` | Error. Unknown control or device. |

## EXAMPLES

### Show all mixer controls

```bash
$ amixer scontrols
```

### Show contents of all controls

```bash
$ amixer scontents
```

### Set Master volume to 50%

```bash
$ amixer sset Master 50%
```

### Set Master volume and unmute

```bash
$ amixer sset Master 50% unmute
```

### Mute Master

```bash
$ amixer sset Master mute
```

### Get specific control value

```bash
$ amixer sget Master
```

### Set volume on specific card

```bash
$ amixer -c 0 sset Master 75%
```

## NOTES

- Uses `~/.asoundrc` or system `/etc/asound.conf` to resolve card names/defaults.
- Output displays control IDs, limit ranges, and current percentage/dB values.

## SEE ALSO

**alsactl**(1), **aplay**(1), **arecord**(1), **alsamixer**(1)

## PACKAGE

`alsa-utils`
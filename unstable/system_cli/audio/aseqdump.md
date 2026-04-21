# aseqdump - ALSA sequencer event dumper

## NAME

**aseqdump** - Dump ALSA sequencer events to standard output in real-time.

## SYNOPSIS

```bash
aseqdump [options]
```

## DESCRIPTION

**aseqdump** dumps ALSA sequencer events to standard output in real-time. It is extremely useful for debugging MIDI connections or verifying physical controller outputs.

## OPTIONS

### Port Options

| Option | Description |
|--------|-------------|
| `-l`, `--list` | List input ports. |
| `-p`, `--port` | Source port(s) to dump from. |

### View Options

| Option | Description |
|--------|-------------|
| `-R`, `--raw-view` | Show raw values (default). |
| `-N`, `--normalized-view` | Show normalized values. |
| `-P`, `--percent-view` | Show percent values. |

### MIDI Version Options

| Option | Description |
|--------|-------------|
| `-u`, `--ump` | Set client MIDI version (0=legacy, 1=UMP MIDI 1.0, 2=UMP MIDI 2.0). Default: `0` |
| `-r`, `--raw` | Do not convert UMP and legacy events. |

### Help

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |
| `-V`, `--version` | Show version. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Exited with SIGINT or port list execution. |
| `1` | Error. Port unavailable. |

## EXAMPLES

### List sequencer input ports

```bash
$ aseqdump -l
```

### Dump MIDI events from port

```bash
$ aseqdump -p 20:0
```

### Dump with normalized values

```bash
$ aseqdump -p 20:0 -N
```

### Dump with percent values

```bash
$ aseqdump -p 20:0 -P
```

### Dump from multiple ports

```bash
$ aseqdump -p 20:0,21:0
```

## NOTES

- Outputs continuous lines representing MIDI Note On, Note Off, Control Change, Pitch Bend, and SysEx events as they arrive.
- View type formatting options: raw (-R), normalized (-N), or percent (-P).

## SEE ALSO

**aconnect**(1), **aplaymidi**(1), **arecordmidi**(1), **aseqsend**(1)

## PACKAGE

`alsa-utils`
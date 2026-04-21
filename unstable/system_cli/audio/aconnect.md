# aconnect - ALSA sequencer connection manager

## NAME

**aconnect** - Manage ALSA sequencer connections between MIDI ports.

## SYNOPSIS

```bash
aconnect [-options] sender receiver
aconnect -i|-o [-options]
aconnect -x
```

## DESCRIPTION

**aconnect** is the ALSA sequencer connection manager. It is used to connect and disconnect ports on the ALSA sequencer routing graph, or to list available input/output ports and their current connections.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-d`, `--disconnect` | Disconnect the specified connection. |
| `-e`, `--exclusive` | Make the connection exclusive. |
| `-r`, `--real` | Convert real-time-stamp on queue. |
| `-t`, `--tick` | Convert tick-time-stamp on queue. |
| `-i`, `--input` | List input (readable) ports. |
| `-o`, `--output` | List output (writable) ports. |
| `-a`, `--all` | Show inactive ports when listing. |
| `-l`, `--list` | List current connections of each port. |
| `-x`, `--removeall` | Remove all exported connections. |

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `sender` | Sender port address in format `client:port` (e.g., `128:0`) |
| `receiver` | Receiver port address in format `client:port` (e.g., `129:0`) |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The operation completed successfully. |
| `1` | Error. Incorrect arguments or ALSA connection failure. |

## EXAMPLES

### List input ports

```bash
$ aconnect -i
```

### List output ports

```bash
$ aconnect -o
```

### Connect two ports

```bash
$ aconnect 128:0 129:0
```

### Disconnect two ports

```bash
$ aconnect -d 128:0 129:0
```

### List current connections

```bash
$ aconnect -l
```

### Remove all connections

```bash
$ aconnect -x
```

## NOTES

- Interfaces directly with ALSA sequencer core.
- No specific environment variables are required.

## SEE ALSO

**aplaymidi**(1), **arecordmidi**(1), **aseqdump**(1), **aseqnet**(1)

## PACKAGE

`alsa-utils`
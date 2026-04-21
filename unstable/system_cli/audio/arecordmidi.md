# arecordmidi - ALSA MIDI file recorder

## NAME

**arecordmidi** - Record MIDI events from ALSA sequencer ports to a standard MIDI file.

## SYNOPSIS

```bash
arecordmidi [options] outputfile
```

## DESCRIPTION

**arecordmidi** is a command-line standard MIDI file recorder. It listens to a specified ALSA sequencer port and records incoming MIDI events into a `.mid` file.

## OPTIONS

### Port Options

| Option | Description |
|--------|-------------|
| `-l`, `--list` | List input ports. |
| `-p`, `--port` | Source port(s) to record from (client:port,...). |

### Timing Options

| Option | Description |
|--------|-------------|
| `-b`, `--bpm` | Tempo in beats per minute. Default: `120` |
| `-f`, `--fps` | Resolution in frames per second (SMPTE). |
| `-t`, `--ticks` | Resolution in ticks per beat or frame. Default: `384` |
| `-i`, `--timesig` | Time signature. Default: `4:4` |

### Recording Options

| Option | Description |
|--------|-------------|
| `-s`, `--split-channels` | Create a track for each channel. |
| `-m`, `--metronome` | Play a metronome signal on specific port. |
| `-n`, `--num-events` | Fixed number of events to record, then exit. |

### Help

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |
| `-V`, `--version` | Show version. |

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `outputfile` | Output MIDI file path. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The MIDI was successfully recorded to the output file. |
| `1` | Error. Cannot connect to port or cannot write to file. |

## EXAMPLES

### List input ports

```bash
$ arecordmidi -l
```

### Record from port to file

```bash
$ arecordmidi -p 128:0 song.mid
```

### Record from multiple ports

```bash
$ arecordmidi -p 128:0,129:0 song.mid
```

### Record with specific tempo

```bash
$ arecordmidi -p 128:0 -b 140 song.mid
```

### Record with split channels

```bash
$ arecordmidi -p 128:0 -s song.mid
```

### Record fixed number of events

```bash
$ arecordmidi -p 128:0 -n 1000 song.mid
```

## NOTES

- Connects to the ALSA sequencer subsystem natively.
- Writes standard MIDI files to the specified output file.

## SEE ALSO

**aplaymidi**(1), **aconnect**(1), **amidi**(1)

## PACKAGE

`alsa-utils`
# aplaymidi - ALSA MIDI file player

## NAME

**aplaymidi** - Play standard MIDI files to ALSA sequencer ports.

## SYNOPSIS

```bash
aplaymidi -p client:port[,...] [-d delay] midifile ...
aplaymidi -l
```

## DESCRIPTION

**aplaymidi** is a command-line MIDI player for ALSA. It sends standard MIDI Files to an ALSA sequencer port.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-l`, `--list` | List all possible output ports. |
| `-p`, `--port` | Set port(s) to play to (required for playback). |
| `-u`, `--ump` | UMP output (only version=1 is supported). |
| `-d`, `--delay` | Delay after song ends in seconds. |

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `midifile` | Standard MIDI file(s) to play. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Finished playing the MIDI file. |
| `1` | Error. Port unavailable or file invalid. |

## EXAMPLES

### List all MIDI output ports

```bash
$ aplaymidi -l
```

### Play a MIDI file to port

```bash
$ aplaymidi -p 14:0 song.mid
```

### Play to multiple ports

```bash
$ aplaymidi -p 14:0,15:0 song.mid
```

### Play with delay after song ends

```bash
$ aplaymidi -p 14:0 -d 2 song.mid
```

## NOTES

- Uses standard ALSA sequencer routing.
- The `-p` option is required for playback.
- Multiple ports can be specified as a comma-separated list.

## SEE ALSO

**arecordmidi**(1), **aconnect**(1), **amidi**(1)

## PACKAGE

`alsa-utils`
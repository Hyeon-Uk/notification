# aseqsend - ALSA sequencer event sender

## NAME

**aseqsend** - Send ALSA sequencer events via command line.

## SYNOPSIS

```bash
aseqsend [options]
```

## DESCRIPTION

**aseqsend** sends a single ALSA sequencer event via command line. It is useful in scripts to emit specific MIDI notes, control changes, or program changes into the ALSA MIDI stream instantly.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |
| `-dest` | Destination client:port to emit event to. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The event was successfully routed. |
| `1` | Error. Device not found or bad formatting. |

## EXAMPLES

### Show help

```bash
$ aseqsend -h
```

### Send event to destination port

```bash
$ aseqsend -dest 128:0
```

## NOTES

- Uses internal ALSA sequencer subsystem directly.
- Silent output when an event is successfully sent.
- Errors are printed to stderr on failure.
- For full explicit options, use `-h` to see event type flags (Note On/Off, Control, etc.).

## SEE ALSO

**aconnect**(1), **aseqdump**(1), **aplaymidi**(1), **amidi**(1)

## PACKAGE

`alsa-utils`
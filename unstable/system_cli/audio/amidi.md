# amidi - ALSA RawMIDI utility

## NAME

**amidi** - Read from and write to ALSA RawMIDI ports.

## SYNOPSIS

```bash
amidi [options]
```

## DESCRIPTION

**amidi** is a command-line utility to read from and write to ALSA RawMIDI ports. It is used to send SysEx instructions to MIDI devices or dump incoming MIDI messages for debugging.

## OPTIONS

### Listing Options

| Option | Description |
|--------|-------------|
| `-l`, `--list-devices` | List all hardware ports. |
| `-L`, `--list-rawmidis` | List all RawMIDI definitions. |

### Port Selection

| Option | Description |
|--------|-------------|
| `-p`, `--port` | Select port by name. |

### Send/Receive Options

| Option | Description |
|--------|-------------|
| `-s`, `--send` | Send the contents of a (.syx) file. |
| `-S`, `--send-hex` | Send hexadecimal bytes. |
| `-r`, `--receive` | Write received data into a file. |
| `-d`, `--dump` | Print received data as hexadecimal bytes. |

### Timeout

| Option | Description |
|--------|-------------|
| `-t`, `--timeout` | Exit when no data received for N seconds. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Data was sent/received or ports listed successfully. |
| `1` | Error. Device busy, port not found, or syntax error. |

## EXAMPLES

### List all MIDI ports

```bash
$ amidi -l
```

### Dump incoming MIDI events

```bash
$ amidi -p hw:1,0,0 --dump
```

### Send SysEx file to device

```bash
$ amidi -p hw:1,0,0 -s patch.syx
```

### Send hexadecimal bytes

```bash
$ amidi -p hw:1,0,0 -S "F0 7E 7F 09 01 F7"
```

### Receive MIDI data to file

```bash
$ amidi -p hw:1,0,0 -r received.syx -t 10
```

## NOTES

- Relies on system ALSA setup and attached MIDI hardware.
- Port listings are plaintext tabulations.
- `--dump` outputs streaming hex values.
- `--receive` writes raw binary MIDI data.

## SEE ALSO

**aconnect**(1), **aplaymidi**(1), **arecordmidi**(1)

## PACKAGE

`alsa-utils`
# alsaucm - ALSA Use Case Manager

## NAME

**alsaucm** - Query and set ALSA audio use cases.

## SYNOPSIS

```bash
alsaucm [options] [command]
```

## DESCRIPTION

**alsaucm** is the ALSA Use Case Manager. It allows applications to query and set complex audio use cases (like "play audio", "voice call") without knowing the underlying mixer controls. It interacts with ALSA's UCM configuration files.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |
| `-c`, `--card` | Open card NAME. |
| `-i`, `--interactive` | Interactive mode (REPL). |
| `-b`, `--batch` | Batch mode; process commands from FILE or `-` for stdin. |
| `-n`, `--no-open` | Do not open the first card found by default. |

## COMMANDS

| Command | Description |
|---------|-------------|
| `open NAME` | Open ALSA card NAME. |
| `reset` | Reset sound card to default state. |
| `reload` | Reload use case configuration. |
| `listcards` | List available UCM cards. |
| `dump FORMAT` | Dump all config information in `text` or `json`. |
| `get IDENTIFIER` | Retrieve specific UCM identifier. |
| `geti IDENTIFIER` | Retrieve specific UCM integer identifier. |
| `set IDENTIFIER VALUE` | Set specific UCM identifier to value. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The operation completed successfully. |
| `1` | Error. Bad arguments or missing UCM profiles. |

## EXAMPLES

### List all UCM cards

```bash
$ alsaucm listcards
```

### Open a specific card

```bash
$ alsaucm -c hw:0
```

### Set use case verb

```bash
$ alsaucm -c bdwi2smic set _verb HiFi
```

### Dump configuration in JSON format

```bash
$ alsaucm dump json
```

### Interactive mode

```bash
$ alsaucm -i
```

### Batch mode from stdin

```bash
$ echo "listcards" | alsaucm -b -
```

## NOTES

- Uses ALSA UCM config files located at `/usr/share/alsa/ucm/` or `/usr/share/alsa/ucm2/`.
- UCM profiles define device-specific audio configurations.

## SEE ALSO

**alsactl**(1), **amixer**(1), **aplay**(1)

## PACKAGE

`alsa-utils`
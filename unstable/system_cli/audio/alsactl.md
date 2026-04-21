# alsactl - ALSA control program

## NAME

**alsactl** - Save, restore, and initialize ALSA soundcard driver settings.

## SYNOPSIS

```bash
alsactl [options] command [card]
```

## DESCRIPTION

**alsactl** is the Advanced Linux Sound Architecture control program. It is used to initialize, store, and restore ALSA soundcard driver settings. It can run as a one-shot command or as a daemon to periodically persist ALSA control states.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help. |
| `-d`, `--debug` | Debug mode. |
| `-v`, `--version` | Print version of this program. |
| `-f`, `--file` | Configuration file to use. Default: `/var/lib/alsa/asound.state` |
| `-l`, `--lock` | Use file locking to serialize concurrent access. |
| `-F`, `--force` | Try to restore the matching controls as much as possible. |
| `-g`, `--ignore` | Ignore 'No soundcards found' error. |
| `-p`, `--period` | Store period in seconds for the daemon command. Default: `300` |
| `-e`, `--pid-file` | Pathname for the process id (daemon mode). Default: `/var/run/alsactl.pid` |
| `-b`, `--background` | Run daemon in background. |
| `-s`, `--syslog` | Use syslog for messages. |

## COMMANDS

| Command | Description |
|---------|-------------|
| `store <card>` | Save current driver setup for one or each soundcard to the configuration file. |
| `restore <card>` | Load current driver setup from the configuration file. |
| `nrestore <card>` | Restore and notify the daemon to rescan. |
| `init <card>` | Initialize driver to a default state. |
| `daemon <card>` | Store state periodically. |
| `rdaemon <card>` | Store state periodically with an initial restore. |
| `kill <cmd>` | Notify daemon to quit, rescan, or save_and_quit. |
| `monitor <card>` | Monitor control events. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The command finished successfully. |
| `1` | Error. No device found, lock errors, or parsing errors on state files. |

## EXAMPLES

### Save current settings

```bash
$ alsactl store
```

### Save settings for a specific card

```bash
$ alsactl store 0
```

### Restore settings

```bash
$ alsactl restore
```

### Initialize card to default state

```bash
$ alsactl init 0
```

### Run as daemon in background

```bash
$ alsactl daemon -b -s
```

### Monitor control events

```bash
$ alsactl monitor 0
```

## NOTES

- Modifies hardware states and persists them to `asound.state`.
- Default state file: `/var/lib/alsa/asound.state`.
- Operates silently unless debug mode is enabled.

## SEE ALSO

**amixer**(1), **aplay**(1), **arecord**(1)

## PACKAGE

`alsa-utils`
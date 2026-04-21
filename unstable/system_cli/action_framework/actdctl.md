# NAME

actdctl - Control utility for the Tizen Activation Daemon

# SYNOPSIS

```bash
actdctl [OPTIONS]
```

# DESCRIPTION

`actdctl` is a control utility for the Tizen Activation Daemon (`actd`). When invoked as `actdctl`, it runs in control mode and dumps the activation daemon's database content. This utility allows inspecting the current state of the activation system.

# OPTIONS

| Option | Long Option | Argument | Description |
|--------|-------------|----------|-------------|
| `-D` | - | - | Set log level to debug |
| `-c` | `--config-file` | `<file>` | Read epc configuration from specified file |
| `-h` | `--help` | - | Display help message and exit |
| - | `--log-level` | `<level>` | Set log level to specified level |

# OUTPUT FORMAT

- **Format:** JSON
- **Structure Details:** Outputs the current database content from the activation daemon in JSON format

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Database dump completed successfully |
| -1 | Error - Failed to initialize modules or dump database |

# FILES

- `/etc/epc/config.json` - Default configuration file location

# EXAMPLES

## Dump activation daemon database

```bash
$ actdctl
```

## Dump with debug logging enabled

```bash
$ actdctl -D
```

## Use custom configuration file

```bash
$ actdctl -c /path/to/custom/config.json
```

## Set specific log level

```bash
$ actdctl --log-level debug
```

# SEE ALSO

actd(8), systemd(1)

# PACKAGE

activationd
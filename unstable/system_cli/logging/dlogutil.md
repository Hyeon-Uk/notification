# dlogutil - Tizen logging utility

## NAME

**dlogutil** - Command-line utility to view and manage Tizen system logs.

## SYNOPSIS

```bash
dlogutil [options] [filterspecs]
```

## DESCRIPTION

**dlogutil** is a command-line utility for viewing and managing Tizen system logs. It provides access to the system's ring buffer logs, allowing users to filter, dump, and save log messages from various components and applications.

## OPTIONS

### General Options

| Option | Description |
|--------|-------------|
| `-s` | Set default filter to silent. Like specifying filterspec '*:s'. |
| `-f <filename>` | Log to file. Default to stdout. |
| `-p <pid1>,<2>,...,<N>` | Pid filter; Print only log with same pid with given one. |
| `-r [<kbytes>]` | Rotate log every kbytes. (16 if unspecified). Requires -f. |
| `-n <count>` | Sets max number of rotated logs to <count>, default 4. |
| `-v <format>` | Sets the log print format. |

### Format Options

| Format | Description |
|--------|-------------|
| `brief` | Brief format (default). |
| `process` | Process format. |
| `tag` | Tag format. |
| `thread` | Thread format. |
| `raw` | Raw format. |
| `time` | Time format. |
| `threadtime` | Thread time format. |
| `long` | Long format. |

### Log Management Options

| Option | Description |
|--------|-------------|
| `-c` | Clear (flush) the entire log and exit. Conflicts with '-g'. |
| `-d` | Dump the log and then exit (don't block). |
| `-t <count>` | Print only the most recent <count> lines (implies -d). |
| `-g` | Get the size of the log's ring buffer and exit. Conflicts with '-c'. |
| `-q <kbytes>` | Set the buffer size in kbytes used by sink thread. |
| `-b <buffer>` | Request alternate ring buffer ('main' (default), 'radio', 'system'). |

### Time Filter Options

| Option | Description |
|--------|-------------|
| `--start-from=<time>` | Set time to start to read. |
| `--end-to=<time>` | Set time to stop to read. |

**Time formats:**
- **Absolute time**: Time value in absolute expression (system clock). Example: `'100'`, `'123.4'`
- **Relative time**: Use macro 'now' for relative expression. Example: `'now - 5'`, `'now + 5'`

## FILTERSPECS

Filterspecs are a series of `<tag>[:priority]` where `<tag>` is a log component tag (or `*` for all).

### Priority Levels

| Level | Description |
|-------|-------------|
| `V` | Verbose |
| `D` | Debug |
| `I` | Info |
| `W` | Warn |
| `E` | Error |
| `F` | Fatal |
| `S` | Silent (suppress all output) |

**Defaults:**
- `*` means `*:D`
- `<tag>` by itself means `<tag>:V`
- If no filterspec is found, filter defaults to `*:D`

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### View all logs

```bash
$ dlogutil
```

### View last 50 lines

```bash
$ dlogutil -t 50
```

### View logs for specific tag

```bash
$ dlogutil MyApp
$ dlogutil MyApp:D
```

### View errors only

```bash
$ dlogutil *:E
```

### View warnings and errors for specific tag

```bash
$ dlogutil MyApp:W
```

### Dump logs and exit

```bash
$ dlogutil -d
```

### Clear all logs

```bash
$ dlogutil -c
```

### Get log buffer size

```bash
$ dlogutil -g
```

### Save logs to file

```bash
$ dlogutil -f /tmp/logs.txt
```

### Filter by PID

```bash
$ dlogutil -p 1234,5678
```

### Use time format output

```bash
$ dlogutil -v time
```

### Use threadtime format

```bash
$ dlogutil -v threadtime
```

### Filter logs by time range

```bash
$ dlogutil --start-from='now - 10' --end-to='now'
$ dlogutil --start-from='100.0' --end-to='100.500'
```

### View system buffer

```bash
$ dlogutil -b system
```

### Multiple filters

```bash
$ dlogutil MyApp:D *:S
```

## NOTES

- Tizen uses dlogutil for system logging (not journalctl).
- The ring buffer has limited size; older logs may be overwritten.
- Use `-d` for non-blocking log retrieval.
- Use `-v threadtime` for detailed thread timing information.
- Combine with grep for more complex filtering.

## SEE ALSO

**dlogctl**(1), **dlogsend**(1)

## PACKAGE

`dlogutil`
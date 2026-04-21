# NAME

alarmmgr_tool - Tizen Alarm Manager command-line interface

# SYNOPSIS

```bash
alarmmgr_tool [OPTIONS]
```

# DESCRIPTION

`alarmmgr_tool` is the command-line interface for Tizen's Alarm Manager service. It provides two functions: setting the system time with propagation delay compensation, and dumping the list of currently registered alarms for debugging and inspection.

# OPTIONS

| Option | Long Option | Argument | Description |
|--------|-------------|----------|-------------|
| `-s` | `--set-time-propagation` | `<params>` | Change system time with propagation delay compensation |
| `-d` | `--dump` | - | Display all registered alarms |

## Set Time Propagation Arguments

The `-s` option requires a comma-separated parameter string:

```
sec=<seconds>,nsec=<nanoseconds>,reqsec=<req_seconds>,reqnsec=<req_nanoseconds>
```

| Parameter | Description |
|-----------|-------------|
| `sec` | Current time seconds |
| `nsec` | Current time nanoseconds |
| `reqsec` | Request time seconds |
| `reqnsec` | Request time nanoseconds |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** `-d` outputs a list of registered alarms with their IDs, trigger times, and associated applications

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Time set or dump completed successfully |
| 1 | Error - Invalid arguments or daemon communication failure |

# ENVIRONMENT

- Communicates with the Alarm Manager daemon via D-Bus
- Alarm data is stored in the alarm database managed by the system service

# EXAMPLES

## Dump all registered alarms

```bash
$ alarmmgr_tool -d
```

## Set system time with propagation delay

```bash
$ alarmmgr_tool -s sec=123456789,nsec=987654321,reqsec=11223344,reqnsec=55667788
```

# SEE ALSO

alarm(3), dbus(1)

# PACKAGE

alarm-manager
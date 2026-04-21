# tlm-launcher

## NAME

tlm-launcher - Tizen Login Manager session script launcher

## SYNOPSIS

```
tlm-launcher -f script_file
tlm-launcher -h
```

## DESCRIPTION

tlm-launcher is a session script execution tool for Tizen Login Manager (tlm). It reads commands from a specified script file and executes them sequentially. It is used to start and monitor processes needed during session initialization.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-f, --file FILE` | Path to the script file to execute |
| `-h, --help` | Show help |

## SCRIPT FILE FORMAT

The script file specifies one command per line, with control characters indicating how the command should be executed.

### Control Characters

| Character | Description |
|-----------|-------------|
| `M` | Execute command via fork & exec and monitor the child process |
| `L` | Execute command (without monitoring) |
| `W` | Wait until specified socket/file is ready |
| `#` | Comment (ignored) |

### Script File Example

```
# Session startup script
M: /usr/bin/DBUS-daemon --system
W: /run/dbus/system_bus_socket
M: /usr/bin/systemd --user
L: /usr/bin/app-launcher
```

## EXAMPLES

Start session with a script file:
```
$ tlm-launcher -f /etc/tlm/session.script
```

Example session script:
```
# Start and monitor DBUS daemon
M: dbus-daemon --system --fork

# Wait for socket to be ready
W: /run/dbus/system_bus_socket

# Execute application (without monitoring)
L: /usr/bin/launcher-app

# Monitor user session service
M: /usr/bin/session-service
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred |

## NOTES

- tlm-launcher terminates when all monitored child processes (M:) have terminated
- Socket/file wait (W:) blocks until the specified file is created
- Lines starting with `#` are treated as comments

## SEE ALSO

- tlm(8)
- tlm-client(1)
- tlm-sessiond(8)

## PACKAGE

tlm
# tlm-client

## NAME

tlm-client - Tizen Login Manager client tool

## SYNOPSIS

```
tlm-client [OPTION...]
```

## DESCRIPTION

tlm-client is a command-line client tool for Tizen Login Manager (tlm). It requests user login, logout, and user switching functions to the tlm daemon via D-Bus.

## OPTIONS

### Main Options

| Option | Description |
|--------|-------------|
| `-l, --login-user` | User login (username, password, seat required) |
| `-o, --logout-user` | User logout (seat required) |
| `-s, --switch-user` | User switching (username, password, seat required) |
| `-r, --run-daemon` | Run tlm daemon (by default, daemon is not started) |

### User Options

| Option | Description | Example |
|--------|-------------|---------|
| `--username` | User name | `user1` |
| `--password` | User password | `mypass` |
| `--seat` | User seat ID | `seat0` |
| `--env` | User environment variables (key-value pairs) | `key1 val1 key2 val2` |

## EXAMPLES

User login:
```
$ tlm-client -l --username user1 --password mypass --seat seat0
```

User login with environment variables:
```
$ tlm-client -l --username user1 --password mypass --seat seat0 --env DISPLAY :0 LANG en_US.UTF-8
```

User logout:
```
$ tlm-client -o --seat seat0
```

User switching:
```
$ tlm-client -s --username user2 --password pass2 --seat seat0
```

Run with daemon (requires root privilege):
```
$ tlm-client -r -l --username user1 --password mypass --seat seat0
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred |

## NOTES

- The tlm daemon must be running
- The `-r` option requires root privilege
- Communicates with tlm daemon via D-Bus

## SEE ALSO

- tlm(8)
- tlm-launcher(1)
- tlm-sessiond(8)

## PACKAGE

tlm
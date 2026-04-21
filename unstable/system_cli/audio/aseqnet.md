# aseqnet - ALSA sequencer network client/server

## NAME

**aseqnet** - Route ALSA sequencer events over TCP network connections.

## SYNOPSIS

```bash
aseqnet [options]              # Server mode
aseqnet [options] server_host  # Client mode
```

## DESCRIPTION

**aseqnet** is an ALSA sequencer network client/server utility. It allows routing MIDI/ALSA sequencer events over IPv4 or IPv6 TCP network connections between machines, enabling multi-machine sound routing.

## OPTIONS

### Network Options

| Option | Description |
|--------|-------------|
| `-6`, `--ipv6` | Use IPv6 TCP protocol. |
| `-p`, `--port` | Specify TCP port (digit or service name). Default: `128` |

### Port Options

| Option | Description |
|--------|-------------|
| `-s`, `--source` | Read from given addr (client:port). |
| `-d`, `--dest` | Write to given addr (client:port). |
| `-n`, `--name` | Use a specific MIDI process name. |

### Output Options

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Print verbose messages. |
| `-i`, `--info` | Print certain received events. |

## MODES

| Mode | Description |
|------|-------------|
| **Server Mode** | Run without `server_host`. Listens on port (default 128) and exposes an ALSA port. |
| **Client Mode** | Run with `server_host`. Connects to a remote `aseqnet` server and creates a linked ALSA port. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Exited cleanly. |
| `1` | Error. Connection refused, port in use, or ALSA configuration failure. |

## EXAMPLES

### Start server on default port

```bash
$ aseqnet
```

### Start server on specific port

```bash
$ aseqnet -p 9000
```

### Connect as client to server

```bash
$ aseqnet 192.168.1.100
```

### Connect to server on specific port

```bash
$ aseqnet -p 9000 192.168.1.100
```

### Connect with verbose output

```bash
$ aseqnet -v 192.168.1.100
```

### Use IPv6

```bash
$ aseqnet -6 2001:db8::1
```

## NOTES

- Uses TCP networking and ALSA sequencer API natively.
- Prints connect/disconnect events; quiet otherwise.
- Use `-v` or `-i` for detailed output.

## SEE ALSO

**aconnect**(1), **aseqdump**(1), **aplaymidi**(1)

## PACKAGE

`alsa-utils`
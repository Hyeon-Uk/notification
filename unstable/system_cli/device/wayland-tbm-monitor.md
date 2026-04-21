# wayland-tbm-monitor - Wayland TBM resource monitor

## NAME

**wayland-tbm-monitor** - Monitor and debug Wayland TBM (Tizen Buffer Manager) resources.

## SYNOPSIS

```bash
wayland-tbm-monitor -list
wayland-tbm-monitor -show <target> [<path>]
wayland-tbm-monitor -trace <cmd> [<resource>] [<target>]
wayland-tbm-monitor -dump_snapshot [<target>] [<scale>] [path]
wayland-tbm-monitor -dump_queue <cmd> [<target>] [<scale>]
wayland-tbm-monitor -debug_level [<target>] <debug_level>
```

## DESCRIPTION

**wayland-tbm-monitor** is a monitoring and debugging tool for Wayland TBM (Tizen Buffer Manager) resources. It provides functionality to list clients, show buffer information, trace resource changes, dump surface snapshots, and set debug levels.

TBM is a protocol for graphics memory management in Tizen, providing buffer allocation and management for Wayland compositors and clients.

## COMMANDS

### List TBM Clients

| Command | Description |
|---------|-------------|
| `-list` | Get list of TBM clients. |

### Show TBM Information

| Command | Description |
|---------|-------------|
| `-show <target> [<path>]` | Print all tbm_surface/tbm_bo information. |

**Target options:**
- `all` - Clients and server
- `server,<pid>,...` - List of targets by PID

**Path options:**
- `console` - Output to console
- `dlog` - Output to dlog
- `<filepath>` - Output to file

### Trace TBM Resources

| Command | Description |
|---------|-------------|
| `-trace on \| off \| status [<resource>] [<target>]` | Control resource tracing. |

**Resource types:**
- `tbm_bo` - TBM buffer object resource trace
- `tbm_surface_internal` - Internal surface resource trace
- `tbm_surface` - TBM surface resource trace
- `tbm_surface_queue` - Surface queue resource trace
- `wl_tbm` - Wayland TBM resource trace
- `all` - All resources trace

### Dump Snapshots

| Command | Description |
|---------|-------------|
| `-dump_snapshot [<target>] [<scale>] [path]` | Make dumps of all tbm_surfaces. |

### Dump Queue

| Command | Description |
|---------|-------------|
| `-dump_queue on \| off \| status [<target>] [<scale>]` | Control dump of queued surfaces. |

### Debug Level

| Command | Description |
|---------|-------------|
| `-debug_level [<target>] <debug_level>` | Set the debug level. |

**Debug levels:**
- `error` - Show error level only
- `warning` - Show warning and error level
- `info` - Show information, warning and error level
- `debug` - Show all debug levels

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### List TBM clients

```bash
$ wayland-tbm-monitor -list
```

### Show TBM information for all

```bash
$ wayland-tbm-monitor -show all
```

### Show TBM information for specific PIDs

```bash
$ wayland-tbm-monitor -show server,1,2 console
$ wayland-tbm-monitor -show server tmp.txt
```

### Enable tracing

```bash
$ wayland-tbm-monitor -trace on tbm_bo server,1,2
$ wayland-tbm-monitor -trace off all
$ wayland-tbm-monitor -trace status
```

### Dump surface snapshots

```bash
$ wayland-tbm-monitor -dump_snapshot all scale=0.5
$ wayland-tbm-monitor -dump_snapshot server,1,2 scale=0.5 /tmp
```

### Enable queue dump

```bash
$ wayland-tbm-monitor -dump_queue on all scale=0.5
$ wayland-tbm-monitor -dump_queue off
```

### Set debug level

```bash
$ wayland-tbm-monitor -debug_level error server
$ wayland-tbm-monitor -debug_level debug server,1,2
```

## NOTES

- TBM (Tizen Buffer Manager) provides graphics memory management for Tizen.
- Trace logs are shown through dlog with "TBM" tag.
- The tool requires appropriate permissions to access TBM resources.
- Debug level changes affect logging verbosity for troubleshooting.

## SEE ALSO

**dlogutil**(1), **wayland**(7)

## PACKAGE

`libwayland-tbm-client`
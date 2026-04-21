# MINICOREDUMPER

**NAME**

minicoredumper - minimal core dump utility

**SYNOPSIS**

```
minicoredumper PID UID GID SIGNAL TIME HOSTNAME EXENAME CONFIG [-d DIR] [-o OUTPUT] [-s]
```

**DESCRIPTION**

`minicoredumper` is a command-line utility for creating minimal core dumps when applications crash. It captures process state information, stack traces, and selective memory regions in a compact format suitable for crash analysis.

The tool is typically invoked automatically by the crash-manager daemon when a process crashes, rather than being called directly by users.

**ARGUMENTS**

| Argument | Description |
|----------|-------------|
| PID | Process ID of crashed process |
| UID | User ID of crashed process |
| GID | Group ID of crashed process |
| SIGNAL | Signal number that caused crash |
| TIME | Timestamp of crash |
| HOSTNAME | System hostname |
| EXENAME | Name of executable |
| CONFIG | Configuration file path |

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-d DIR` | Temporary directory for dump files |
| `-o OUTPUT` | Output coredump filename |
| `-s` | Skip core dump (metadata only) |

**EXAMPLES**

```
# Typically invoked by crash-manager
minicoredumper 12345 5000 5000 11 1634567890 localhost myapp /etc/minicoredumper/config.json -d /tmp/crash -o core.myapp
```

**SEE ALSO**

crash-manager(8), crash-json2info(1)

**PACKAGE**

External package: minicoredumper
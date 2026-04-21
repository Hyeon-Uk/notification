# NSJAIL

**NAME**

nsjail - process isolation tool using Linux namespaces

**SYNOPSIS**

```
nsjail [OPTIONS] -- COMMAND [ARGS...]
```

**DESCRIPTION**

`nsjail` is a command-line utility for isolating processes using Linux namespaces, cgroups, and seccomp-bpf filters. It provides a secure sandboxing mechanism for running untrusted code with restricted system access.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `--mode MODE` | Execution mode (o: once, r: restart, e: exec) |
| `--chroot DIR` | Chroot directory |
| `--user USER` | Run as user |
| `--group GROUP` | Run as group |
| `--rlimit_nofile N` | Limit number of open files |
| `--time_limit N` | Time limit in seconds |
| `--log LOGFILE` | Log file path |

**EXAMPLES**

```
# Run command in sandbox
nsjail --chroot /tmp/sandbox -- /bin/sh

# Run with resource limits
nsjail --rlimit_nofile 100 --time_limit 60 -- ./myapp

# Run with user isolation
nsjail --user nobody --group nogroup -- /bin/cat /etc/passwd
```

**SEE ALSO**

nsjail.sh(1)

**PACKAGE**

External package: nsjail
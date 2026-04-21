# pkill Specification

## 1. Overview
- **CLI Path:** `/usr/bin/pkill`
- **Execution Model:** Standard Utility
- **Description:** `pkill` is a standard system utility included essentially from the upstream `procps-ng` package. It offers basic command functionalities.

## 2. Command Syntax
```bash
/usr/bin/pkill [OPTIONS]
```

## 3. Global Options
| Option                        | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| `-<sig>` | signal to send (either number or name) |
| `-H, --require-handler` | match only if signal handler is present |
| `-q, --queue <value>` | integer value to be sent with the signal |
| `-e, --echo` | display what is killed |
| `-c, --count` | count of matching processes |
| `-f, --full` | use full process name to match |
| `-g, --pgroup <PGID,...>` | match listed process group IDs |
| `-G, --group <GID,...>` | match real group IDs |
| `-i, --ignore-case` | match case insensitively |
| `-n, --newest` | select most recently started |
| `-o, --oldest` | select least recently started |
| `-O, --older <seconds>` | select where older than seconds |
| `-P, --parent <PPID,...>` | match only child processes of the given parent |
| `-s, --session <SID,...>` | match session IDs |
| `--signal <sig>` | signal to send (either number or name) |
| `-t, --terminal <tty,...>` | match by controlling terminal |
| `-u, --euid <ID,...>` | match by effective IDs |
| `-U, --uid <ID,...>` | match by real IDs |
| `-x, --exact` | match exactly with the command name |
| `-F, --pidfile <file>` | read PIDs from file |
| `-L, --logpidfile` | fail if PID file is not locked |
| `-r, --runstates <state>` | match runstates [D,S,Z,...] |
| `-A, --ignore-ancestors` | exclude our ancestors from results |
| `--cgroup <grp,...>` | match by cgroup v2 names |
| `--ns <PID>` | match the processes that belong to the same |
| `--nslist <ns,...>` | list which namespaces will be considered for |
| `--env <name=val,...>` | match on environment variable |
| `-h, --help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.

## 6. Output Format
- **Format:** Text (Stdout)
- **Structure Details:** Outputs basic process or file status logs.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: View standard help
**Context:** Check binary usage syntax.
```bash
$ pkill --help
```

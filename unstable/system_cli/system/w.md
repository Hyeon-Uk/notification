# w Specification

## 1. Overview
- **CLI Path:** `/usr/bin/w`
- **Execution Model:** Standard Utility
- **Description:** `w` is a standard system utility included from the upstream `procps-ng` package. It displays information about the users currently on the machine, and their processes. The header shows the current time, how long the system has been running, how many users are currently logged on, and the system load averages.

## 2. Command Syntax
```bash
/usr/bin/w [OPTIONS] [USER]
```

## 3. Global Options
| Option | Description |
|--------|-------------|
| `-h, --no-header` | do not print header |
| `-u, --no-current` | ignore current process username |
| `-s, --short` | short format (omit login time, JCPU, PCPU) |
| `-f, --from` | toggle printing the from (remote host) field |
| `-i, --ip-addr` | display IP address instead of hostname for from field |
| `-o, --old-style` | old style output |
| `--help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.
- Reads system information from `/var/run/utmp` and `/proc`.

## 6. Output Format
- **Format:** Text (Stdout)
- **Structure Details:** 
  - **Header:** Current time, uptime, number of users, load averages
  - **USER:** Login name
  - **TTY:** Terminal name
  - **FROM:** Remote host
  - **LOGIN@:** Login time
  - **IDLE:** Idle time
  - **JCPU:** Total CPU time used by all processes attached to the TTY
  - **PCPU:** CPU time used by the current process
  - **WHAT:** Current process

**Example Output:**
```
 11:30:45 up 10 days,  3:15,  2 users,  load average: 0.05, 0.10, 0.15
USER     TTY        FROM             LOGIN@   IDLE   JCPU   PCPU WHAT
user1    tty1                       09:00    2:15m  0.10s  0.05s -bash
user2    pts/0     192.168.1.100    10:30    1:00   0.20s  0.10s vim file.txt
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: Display logged-in users
**Context:** Check who is logged in and what they are doing.
```bash
$ w
 11:30:45 up 10 days,  3:15,  2 users,  load average: 0.05, 0.10, 0.15
USER     TTY        FROM             LOGIN@   IDLE   JCPU   PCPU WHAT
user1    tty1                       09:00    2:15m  0.10s  0.05s -bash
user2    pts/0     192.168.1.100    10:30    1:00   0.20s  0.10s vim file.txt
```

### Example 2: Display without header
**Context:** Get only user information without the header.
```bash
$ w -h
user1    tty1                       09:00    2:15m  0.10s  0.05s -bash
user2    pts/0     192.168.1.100    10:30    1:00   0.20s  0.10s vim file.txt
```

### Example 3: Display short format
**Context:** Compact view of logged-in users.
```bash
$ w -s
 11:30:45 up 10 days,  3:15,  2 users,  load average: 0.05, 0.10, 0.15
USER     TTY        FROM              IDLE WHAT
user1    tty1                         2:15m -bash
user2    pts/0     192.168.1.100     1:00 vim file.txt
```

### Example 4: Display specific user's information
**Context:** Check what a specific user is doing.
```bash
$ w user1
 11:30:45 up 10 days,  3:15,  2 users,  load average: 0.05, 0.10, 0.15
USER     TTY        FROM             LOGIN@   IDLE   JCPU   PCPU WHAT
user1    tty1                       09:00    2:15m  0.10s  0.05s -bash
```

## 9. Package Information
- **Package:** procps-ng
- **Source:** https://gitlab.com/procps-ng/procps
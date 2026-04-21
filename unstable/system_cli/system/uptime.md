# uptime Specification

## 1. Overview
- **CLI Path:** `/usr/bin/uptime`
- **Execution Model:** Standard Utility
- **Description:** `uptime` is a standard system utility included from the upstream `procps-ng` package. It displays the current time, how long the system has been running, how many users are currently logged on, and the system load averages for the past 1, 5, and 15 minutes.

## 2. Command Syntax
```bash
/usr/bin/uptime [OPTIONS]
```

## 3. Global Options
| Option | Description |
|--------|-------------|
| `-p, --pretty` | show uptime in pretty format |
| `-s, --since` | system up since, in yyyy-mm-dd HH:MM:SS format |
| `-h, --help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.
- Reads system information from `/proc/uptime` and `/proc/loadavg`.

## 6. Output Format
- **Format:** Text (Stdout)
- **Structure Details:** 
  - Current time
  - Uptime duration
  - Number of logged-in users
  - System load averages (1, 5, and 15 minutes)

**Example Output:**
```
 11:30:45 up 10 days,  3:15,  2 users,  load average: 0.05, 0.10, 0.15
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: Display system uptime
**Context:** Check how long the system has been running.
```bash
$ uptime
 11:30:45 up 10 days,  3:15,  2 users,  load average: 0.05, 0.10, 0.15
```

### Example 2: Display uptime in pretty format
**Context:** Human-readable uptime display.
```bash
$ uptime -p
up 1 week, 3 days, 3 hours, 15 minutes
```

### Example 3: Display system start time
**Context:** Check when the system was started.
```bash
$ uptime -s
2026-03-30 08:15:30
```

## 9. Package Information
- **Package:** procps-ng
- **Source:** https://gitlab.com/procps-ng/procps
# resource-monitor Specification

## 1. Overview
- **CLI Path:** `/usr/bin/resource-monitor`
- **Execution Model:** **Streaming** (repeats until count reached or interrupted)
- **Description:** `resource-monitor` provides real-time monitoring of system and per-process resource usage including CPU, memory, and I/O on Tizen. Supports monitoring specific processes by PID or process groups by parent PID.

## 2. Command Syntax
```bash
resource-monitor [options]
```

## 3. Global Options

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `-h` | Boolean | `false` | Show help message. |
| `-d <secs>` | Integer | `1` | Update interval in seconds. |
| `-n <max>` | Integer | infinite | Maximum number of updates. |
| `-p <pid>`, `--pid <pid>` | Integer | — | Monitor a specific process by PID. |
| `-g <ppid>`, `--ppid <ppid>` | Integer | — | Monitor a process group by parent PID. |

## 4. Commands (if applicable)
Single-purpose command, no subcommands.

## 5. Environment Variables & Configuration
- Reads resource data from `/proc/` and Tizen resource monitoring APIs.

## 6. Output Format
- **Format:** Text (stdout)
- **Structure Details:** Periodically updated tabular output with resource metrics.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Monitoring completed or gracefully interrupted. |
| `1` | Error | Invalid PID or permission denied. |

## 8. Practical Examples

### Example 1: Monitor all resources (2-second intervals)
```bash
$ resource-monitor -d 2
```

### Example 2: Monitor a specific process
```bash
$ resource-monitor -p 1234 -n 10
```

### Example 3: Monitor a process group
```bash
$ resource-monitor --ppid 500 -d 5
```

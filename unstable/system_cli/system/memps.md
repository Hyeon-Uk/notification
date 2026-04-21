# memps Specification

## 1. Overview
- **CLI Path:** `/usr/bin/memps`
- **Execution Model:** **One-shot**
- **Description:** `memps` (Memory Process Status) is Tizen's memory analysis tool. It provides detailed memory usage information for all processes or a specific process, including RSS, PSS, shared/private memory breakdowns, and summary statistics.

## 2. Command Syntax
```bash
memps [options] [pid]
```

## 3. Global Options

| Flag | Type | Description |
|------|------|-------------|
| `-a` | Boolean | Show all processes with memory info. |
| `-v` | Boolean | Verbose: show all processes in detail. |
| `-q` | Boolean | Quick verbose: experimental fast detailed view of all processes. |
| `-r` | Boolean | Show RSS of all processes. |
| `-s <pid>` | Integer | Show only sum of each memory category for a specific PID. |
| `-f <path>` | String | Output all process memory info to a file. |

## 4. Commands (if applicable)
Single-purpose command. Pass a `<pid>` without flags to show memory info for that specific process.

## 5. Environment Variables & Configuration
- Reads memory information from `/proc/<pid>/smaps` and `/proc/meminfo`.

## 6. Output Format
- **Format:** Text (stdout or file)
- **Structure Details:** Outputs memory data in tabular format with columns for RSS, PSS, Shared, Private, and other memory metrics.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Memory information retrieved. |
| `1` | Error | PID not found or permission denied. |

## 8. Practical Examples

### Example 1: Show all processes with memory info
```bash
$ memps -a
```

### Example 2: Detailed memory for a specific process
```bash
$ memps -s 1234
```

### Example 3: Dump all process memory to a file
```bash
$ memps -f /tmp/memps_report.txt
```

### Example 4: Quick verbose overview
```bash
$ memps -q
```

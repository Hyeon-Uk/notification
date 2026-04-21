# dlogsend Specification

## 1. Overview
- **CLI Path:** `/usr/bin/dlogsend`
- **Execution Model:** **One-shot**
- **Description:** `dlogsend` injects log messages into DLog ring buffers from the command line. It is used for testing, scripting, and debugging purposes, allowing users to send log entries with a specified priority, tag, and target buffer without using the native DLog API.

## 2. Command Syntax
```bash
dlogsend [-p priority] [-b buffer] [-t tag] [-c number] [-s] [-d time] [-f freq] [message]
```
If `message` is not specified, each line from standard input is logged separately until EOF.

## 3. Global Options

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `-p <priority>` | String | `Info` | Log priority. Values: `Debug`, `Info`, `Warning`, `Error`, `Verbose`, `Fatal`. First letter is sufficient (e.g., `-p D`). |
| `-b <buffer>` | String | `apps` | Target ring buffer: `main`, `system`, `radio`, `apps`. |
| `-b <sink>` | String | — | Alternative DLog interface sinks: `dotnet_api`, `native_api`. |
| `-t <tag>` | String | `DLOG_SEND` | Identification string for filtering. Can be any arbitrary string. |
| `-c <number>` | Integer | `1` | Number of times to repeat each message. |
| `-s` | Boolean | `false` | Collect and print transmission statistics after sending. |
| `-k` | Boolean | `false` | Mark log entry as a 'critical' log. |
| `-z` | Boolean | `false` | Mark log entry as a 'secure' log. |
| `-d <time>` | Float | — | Wait a given number of seconds after sending a log. |
| `-f <freq>` | Integer | — | Only wait after every given amount of logs (used with `-d`). |
| `-h` | Boolean | `false` | Show help message and exit. |

## 4. Commands (if applicable)
Single-purpose command, no subcommands.

## 5. Environment Variables & Configuration
- Uses the DLog UNIX socket interface managed by the `dlog_logger` daemon.
- DLog configuration is defined in `/etc/dlog.conf`.

## 6. Output Format
- **Format:** Silent by default
- **Structure Details:** No output is produced on success. When `-s` (statistics) flag is used, transmission statistics (rate, count) are printed to stdout.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Log message(s) successfully delivered to the ring buffer. |
| `1` | Error | Invalid arguments or DLog socket connection failure. |

## 8. Practical Examples

### Example 1: Send a simple log message
**Context:** Inject a test message into the apps buffer at Info priority.
```bash
$ dlogsend "Hello from test script"
```

### Example 2: Send a debug log with a custom tag
**Context:** Inject a debug message tagged for a specific component.
```bash
$ dlogsend -p D -t MY_MODULE "Initialization complete"
```

### Example 3: Stream logs from stdin
**Context:** Pipe a file's content line-by-line into system log buffer.
```bash
$ cat /tmp/test_cases.txt | dlogsend -b system -t QA_SCRIPT
```

### Example 4: Stress test with statistics
**Context:** Send 1000 repeated messages and display the throughput stats.
```bash
$ dlogsend -c 1000 -s -t STRESS_TEST "load test message"
```

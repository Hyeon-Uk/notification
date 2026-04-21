# crash-json2info Specification

## 1. Overview
- **CLI Path:** `/usr/bin/crash-json2info`
- **Execution Model:** **One-shot**
- **Description:** Converts crash report data from JSON format to human-readable info format. Part of Tizen's crash reporting pipeline.

## 2. Command Syntax
```bash
crash-json2info [-o|--output <output>] <input json>
```

## 3. Global Options

| Flag | Short | Type | Description |
|------|-------|------|-------------|
| `--output` | `-o` | String | Output file path. Omit for stdout. |

## 4. Commands (if applicable)
Single-purpose command.

## 5. Environment Variables & Configuration
- Part of the Tizen crash-worker pipeline.

## 6. Output Format
- **Format:** Text
- **Structure Details:** Human-readable crash report with backtrace.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Conversion completed. |
| `1` | Error | Invalid JSON or file I/O error. |

## 8. Practical Examples

### Example 1: Convert crash JSON
```bash
$ crash-json2info -o /tmp/report.txt /opt/usr/share/crash/report.json
```

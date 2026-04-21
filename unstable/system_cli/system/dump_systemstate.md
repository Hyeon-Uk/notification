# dump_systemstate Specification

## 1. Overview
- **CLI Path:** `/usr/bin/dump_systemstate`
- **Execution Model:** **One-shot**
- **Description:** `dump_systemstate` collects a comprehensive system state report for diagnostics. It aggregates binary version info, Tizen version, kernel version, boot arguments, memory stats, process listings, Buxton configuration, kernel messages, DLog output, journal logs, installed package lists, and additional user-defined extras from configuration files.

## 2. Command Syntax
```bash
dump_systemstate [-b] [-k] [-d] [-j] [-p] [-e] [-E] [-f file]
```
When invoked without flags, the tool outputs the default system state report (includes extras by default).

## 3. Global Options

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `-b` | Boolean | `false` | Include Buxton configuration data in the dump. |
| `-k` | Boolean | `false` | Include kernel messages (`dmesg`). Requires root. |
| `-d` | Boolean | `false` | Include DLog messages in the dump. |
| `-j` | Boolean | `false` | Include journal log messages in the dump. |
| `-p` | Boolean | `false` | Include the list of installed packages in the dump. |
| `-e` | Boolean | `true` | Include extras defined in the configuration (default behavior). |
| `-E` | Boolean | `false` | Exclude extras defined in the configuration. |
| `-f <file>` | String | stdout | Write output to a file instead of stdout. |

## 4. Commands (if applicable)
Single-purpose command, no subcommands.

## 5. Environment Variables & Configuration
- Extras configuration directories:
  - `/etc/dump_systemstate.conf.d/programs` — Additional programs to execute during dump.
  - `/etc/dump_systemstate.conf.d/files` — Additional files to include in the dump.
- System info sources: `/etc/info.ini`, `/etc/tizen-release`, `/proc/version`, `/proc/cmdline`.

## 6. Output Format
- **Format:** Text (stdout or file)
- **Structure Details:** Sections are delimited by `==== <Section Title> (<source>)` headers. Each section contains the relevant system data.

**Example output (partial):**
```text
dump_systemstate: 2026-03-26 23:51:11+0900

==== Binary version (/etc/info.ini)
[Version]
Model=Tizen11.0/Unified;
Build=tizen-unified_20260309.210600;
Release=20260309.210600;

==== Tizen version (/etc/tizen-release)
Tizen11.0/Unified 11.0.0 (x86_64)
VERSION = 11.0.0
CODENAME = TELEVEN

==== Kernel version (/proc/version)
Linux version 4.4.35-x86_64 (root@86e3d21fc4cd) ...

==== Boot arguments (/proc/cmdline)
...
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | System state dump completed successfully. |
| `1` | Error | Permission denied (e.g., `-k` without root) or file write failure. |

## 8. Practical Examples

### Example 1: Default dump to stdout
**Context:** Quickly view the overall system state in the terminal.
```bash
$ dump_systemstate
```

### Example 2: Full dump to a file
**Context:** Capture a comprehensive report with all data sources for later analysis.
```bash
$ dump_systemstate -b -k -d -j -p -f /tmp/system_report.txt
```

### Example 3: Dump without extras
**Context:** Quick dump excluding custom extras for faster output.
```bash
$ dump_systemstate -E
```

### Example 4: Dump with DLog and packages
**Context:** Collect DLog messages and package list for app debugging.
```bash
$ dump_systemstate -d -p -f /tmp/app_debug.txt
```

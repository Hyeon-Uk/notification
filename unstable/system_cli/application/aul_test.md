# NAME

aul_test - Tizen AUL (Application Utility Library) testing utility

# SYNOPSIS

```bash
aul_test <command> [arguments]
```

# DESCRIPTION

`aul_test` is a development and testing utility for the Tizen AUL (Application Utility Library). It provides direct access to low-level AUL API calls for debugging application lifecycle, process management, MIME handling, and window management operations.

# COMMANDS

## Process Management

| Command | Arguments | Description |
|---------|-----------|-------------|
| `add_proc_group` | `<pid>` | Add a process to the process group |
| `foreach_proc_group` | - | List all processes in the process group |
| `bt` | `<pid>` | Print backtrace for a given process |
| `get_app_bypid` | `<pid>` | Get application ID for a given PID |
| `get_appid_bypid_async` | `<pid>` | Asynchronously get application ID by PID |
| `get_last_caller_pid` | `<pid>` | Get the PID of the last caller for a process |
| `get_last_caller_pid_for_uid` | `<pid> <uid>` | Get the last caller PID for a specific UID |
| `get_cpu_usage` | `<pid>` | Get CPU usage for a process |

## Application Lifecycle

| Command | Arguments | Description |
|---------|-----------|-------------|
| `app_is_running_async` | `<appid>` | Check if an application is running (async) |
| `get_app_lifecycle` | `<appid>` | Get the lifecycle state of an application |
| `disable_alias_info` | `<appid>` | Disable alias info for an application |
| `enable_alias_info` | `<appid>` | Enable alias info for an application |

## MIME Handling

| Command | Arguments | Description |
|---------|-----------|-------------|
| `get_defapp_mime` | `<mime_type>` | Get the default application for a MIME type |
| `get_default_app` | `<operation> <uri> <mime>` | Get the default application for a given operation/URI/MIME |
| `get_mime_content` | `<content>` | Get the MIME type from content |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Each command outputs its result as plain text (process info, app IDs, or status messages)

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - The test operation completed successfully |
| 1 | Error - Invalid command, missing arguments, or AUL API failure |

# ENVIRONMENT

- Communicates with the AUL daemon via D-Bus (`org.tizen.aul`)

# EXAMPLES

## Check if an application is running

```bash
$ aul_test app_is_running_async org.tizen.setting
```

## Get app ID from PID

```bash
$ aul_test get_app_bypid 1234
```

## Get CPU usage for a process

```bash
$ aul_test get_cpu_usage 5678
```

## Get default application for MIME type

```bash
$ aul_test get_defapp_mime "image/jpeg"
```

# SEE ALSO

app_launcher(1), aulctl(1), appid2pid(1)

# PACKAGE

aul
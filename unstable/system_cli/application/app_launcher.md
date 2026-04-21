# NAME

app_launcher - Tizen application lifecycle management tool

# SYNOPSIS

```bash
app_launcher [OPTIONS]
```

# DESCRIPTION

`app_launcher` is a core utility of the Tizen Application Framework (AUL). It manages the lifecycle of Tizen applications from the shell, supporting operations to list installed apps, start/terminate/kill applications by App ID, and check running status.

# OPTIONS

| Option | Long Option | Argument | Description |
|--------|-------------|----------|-------------|
| `-h` | `--help` | - | Show help options |
| - | `--help-all` | - | Show all help options including hidden ones |
| - | `--help-option` | - | Show additional options |
| `-l` | `--list` | - | Show installed application list |
| `-S` | `--status` | - | Show the running application information |
| `-s` | `--start` | `<APP_ID>` | Execute the application by APPLICATION ID |
| `-k` | `--kill` | `<APP_ID>` | Kill the running application (sends SIGKILL) |
| `-t` | `--terminate` | `<APP_ID>` | Terminate the running application (sends SIGTERM, allows graceful shutdown) |
| `-r` | `--is-running` | `<APP_ID>` | Check whether the application is running or not |
| `-f` | `--fast-start` | `<APP_ID>` | **[Deprecated]** Execute the application quickly |
| `-e` | `--direct-start` | `<APP_ID>` | Execute the application without the loader |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:**
  - `-l`: Outputs installed apps as `appId` lines
  - `-S`: Outputs running apps in `appId (PID)` format
  - `-s`, `-k`, `-t`: Outputs a result code or error message
  - `-r`: Outputs running status (boolean-like result)

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - The requested operation completed successfully |
| 1 | Error - Application ID not found, operation failed, or insufficient permissions |

# ENVIRONMENT

- Communicates with the AUL (Application Utility Library) daemon via D-Bus
- Application records are stored in the package manager database under `/opt/dbspace/`

# EXAMPLES

## List installed applications

```bash
$ app_launcher -l
```

## Check running status with PIDs

```bash
$ app_launcher -S
```

## Launch an application

```bash
$ app_launcher -s org.tizen.setting
```

## Terminate an application gracefully

```bash
$ app_launcher -t org.tizen.browser
```

## Kill a frozen application

```bash
$ app_launcher -k org.tizen.browser
```

## Check if application is running

```bash
$ app_launcher -r org.tizen.browser
```

# SEE ALSO

aulctl(1), appmgr_tool(1), pkgcmd(1)

# PACKAGE

aul
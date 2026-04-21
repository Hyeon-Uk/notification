# NAME

aul_window - Tizen window management utility for AUL framework

# SYNOPSIS

```bash
aul_window <command> [arguments]
```

# DESCRIPTION

`aul_window` is a Tizen window management utility for the AUL framework. It provides commands to query the window stack, get the focused process, attach/detach windows between applications, and monitor window events in real-time.

# COMMANDS

| Command | Arguments | Description |
|---------|-----------|-------------|
| `foreach_window_stack` | - | Enumerate the current window stack |
| `get_focused_pid` | - | Get the PID of the currently focused window's process |
| `attach_window` | `<parent_appid> <child_appid>` | Attach a child window above the parent window |
| `detach_window` | `<child_appid>` | Detach a previously attached child window |
| `attach_window_below` | `<parent_appid> <child_appid>` | Attach a child window below the parent window |
| `monitor_window_event` | - | Monitor window events in real-time (streaming mode) |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:**
  - `foreach_window_stack`: Lists windows
  - `get_focused_pid`: Outputs a PID integer
  - `monitor_window_event`: Streams events continuously

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Command executed successfully |
| 1 | Error - Invalid command, Wayland not available, or AUL failure |

# ENVIRONMENT

- Requires a running Wayland compositor (Enlightenment) for window operations
- Communicates with AUL daemon via D-Bus

# EXAMPLES

## View the window stack

```bash
$ aul_window foreach_window_stack
```

## Get focused window's PID

```bash
$ aul_window get_focused_pid
```

## Attach a popup window above a host app

```bash
$ aul_window attach_window org.tizen.host_app org.tizen.popup_app
```

## Detach a child window

```bash
$ aul_window detach_window org.tizen.popup_app
```

## Monitor window events in real-time

```bash
$ aul_window monitor_window_event
```

# SEE ALSO

app_launcher(1), aul_test(1)

# PACKAGE

aul
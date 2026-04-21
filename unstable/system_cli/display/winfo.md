# winfo

## NAME

winfo - Enlightenment Window Manager information query tool

## SYNOPSIS

```
winfo [OPTION...] [ARGUMENTS...]
```

## DESCRIPTION

winfo is a command-line tool for querying and controlling various information of the Enlightenment Window Manager. It can check Wayland protocol tracing, compositor object information, FPS measurement, input device information, and more.

## OPTIONS

### Basic Options

| Option | Description |
|--------|-------------|
| `-version` | Display version information |
| `-protocol_trace [console\|file_path\|disable]` | Set protocol tracing |
| `-protocol_rule [COMMAND] [ARG]...` | Manage protocol rules |
| `-eina_log_levels [levels]` | Set eina log levels |
| `-eina_log_path [console\|file_path]` | Set eina log path |
| `-dlog [on:1,off:0]` | Enable/disable dlog |
| `-trace [hwc\|exsync] [off:0,on:1]` | Set tracing |

### Information Query Options

| Option | Description |
|--------|-------------|
| `-compobjs [simple]` | List compositor objects |
| `-connected_clients` | List connected clients |
| `-container_info` | Display container information |
| `-fps [COMMAND] [ARG]...` | FPS information |
| `-hwc_wins` | List HWC windows |
| `-input_devices` | List input devices |
| `-input_region` | Display input region information |
| `-keygrab_status` | Display key grab status |
| `-keymap` | Display keymap information |
| `-module_info` | Display module information |
| `-output_mode [idx] [mode]` | Set output mode |
| `-reslist [-tree\|-p]` | List resources |
| `-screen_info` | Display screen information |
| `-subsurface` | Display subsurface information |

### Protocol Rule Commands

| Command | Description |
|---------|-------------|
| `add [allow\|deny] [RULE]` | Add a rule |
| `remove [all\|INDEX]` | Remove a rule |
| `file [PATH]` | Load rules from file |
| `print` | Print current rules |
| `help` | Display help |

### FPS Commands

| Option | Description |
|--------|-------------|
| `-help` | Display FPS help |
| `-win_id [id]` | Display FPS for specific window |

### Resource List Options

| Option | Description |
|--------|-------------|
| `-tree` | Display all resources in tree format |
| `-p {pid}` | Display client resources for specific PID |

## EXAMPLES

Check version information:
```
$ winfo -version
```

Query connected clients:
```
$ winfo -connected_clients
```

Query screen information:
```
$ winfo -screen_info
```

Query input devices:
```
$ winfo -input_devices
```

Measure FPS:
```
$ winfo -fps
```

Measure FPS for specific window:
```
$ winfo -fps -win_id 12345
```

Enable protocol tracing:
```
$ winfo -protocol_trace console
```

Add a protocol rule:
```
$ winfo -protocol_rule add allow "iface=wl_touch and msg=down"
```

Print current protocol rules:
```
$ winfo -protocol_rule print
```

Query resource list:
```
$ winfo -reslist -tree
```

Query resources for specific process:
```
$ winfo -reslist -p 1234
```

Enable HWC tracing:
```
$ winfo -trace hwc 1
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred |

## SEE ALSO

- enlightenment(1)
- wayland(7)
- weston-info(1)

## PACKAGE

enlightenment
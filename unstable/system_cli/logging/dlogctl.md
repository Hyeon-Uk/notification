# dlogctl Specification

## 1. Overview
- **CLI Path:** `/usr/bin/dlogctl`
- **Execution Model:** **One-shot**
- **Description:** `dlogctl` manages DLog runtime filtering and platform logging configuration. It controls per-tag/priority/PID rate limits, enables or disables logging for specific ring buffers, manages stdout redirection, and configures compression buffers. Requires dynamic DLog configuration to be enabled on the system.

## 2. Command Syntax
```bash
# Filter management mode
dlogctl [-t/--tag tag] [-p/--priority priority] [--pid pid] (-g/--get | -s/--set value | -c/--clear)

# Buffer control mode
dlogctl [-b/--buffer bufname] [-b ...] (--enable | --disable)
dlogctl [-b/--buffer bufname] [-b ...] (--enable-stdout | --disable-stdout)

# Compression control mode
dlogctl --compression-name <name> --compression-resize <size>
```

## 3. Global Options

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `-t, --tag <tag>` | String | `*` | Target log tag for filter operations. |
| `-p, --priority <priority>` | String | `*` | Target priority level: `F`(atal), `E`(rror), `W`(arning), `I`(nfo), `V`(erbose), `D`(ebug), `S`(ilent), `*`(all). |
| `--pid <pid>` | Integer | — | Target process ID. Mutually exclusive with `--tag` and `--priority`. |
| `-g, --get` | Boolean | `false` | Get the current limit for a given tag/priority. If neither is given, dump the entire active filterset. |
| `-s, --set <value>` | String | — | Set a new limit. Values: `allow`, `deny`, or an integer `0–10000` (allowed logs per minute). |
| `-c, --clear` | Boolean | `false` | Clear the limit for a given tag/priority. If neither is given, clear the entire filterset. |
| `-b, --buffer <bufname>` | String | all | Target buffer(s): `main`, `system`, `radio`, `apps`. Can be specified multiple times. |
| `--enable` | Boolean | `false` | Enable platform logging for chosen buffers. |
| `--disable` | Boolean | `false` | Disable platform logging for chosen buffers. |
| `--enable-stdout` | Boolean | `false` | Enable stdout redirection for chosen buffers. |
| `--disable-stdout` | Boolean | `false` | Disable stdout redirection for chosen buffers. |
| `--compression-name <name>` | String | — | Named compression buffer to resize. Must be used with `--compression-resize`. |
| `--compression-resize <size>` | Integer | — | Set compression buffer to the given size in bytes. |

## 4. Commands (if applicable)
Single-purpose command, no subcommands. Operation mode is determined by the flags used.

## 5. Environment Variables & Configuration
- **Required:** Dynamic DLog config must be enabled. The `dynamic_config_path` config entry in `/etc/dlog.conf` must point to a directory path.
- **Important:** Filters do not affect the `apps` buffer by default. Set `limiter_apply_to_all_buffers=1` in config to enable.
- **Note:** Filters are currently per-process; static filters are shadowed by dynamic ones but cannot be overridden at runtime.

## 6. Output Format
- **Format:** Text (stdout)
- **Structure Details:** When using `-g`, outputs the current filter limits as key-value text. Otherwise, outputs a success/failure message.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Filter or buffer control operation completed successfully. |
| `1` | Error | Invalid arguments, dynamic config not enabled, or daemon communication failure. |

## 8. Practical Examples

### Example 1: Dump the entire active filterset
**Context:** View all currently active dynamic log filters.
```bash
$ dlogctl -g
```

### Example 2: Throttle a noisy tag
**Context:** Limit a verbose tag to 100 messages per minute.
```bash
$ dlogctl -t NOISY_MODULE -p D -s 100
```

### Example 3: Disable logging for a specific buffer
**Context:** Stop platform logging for the radio buffer to reduce overhead.
```bash
$ dlogctl -b radio --disable
```

### Example 4: Clear all dynamic filters
**Context:** Remove all user-defined runtime filters.
```bash
$ dlogctl -c
```

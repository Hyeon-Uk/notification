# vconftool - Buxton wrapper tool for vconf APIs

## NAME

**vconftool** - Command-line tool for managing key-value configuration data through Buxton2 backend.

## SYNOPSIS

```bash
vconftool set [set-options] <key name> <value>
vconftool get [get-options] <key name>
vconftool unset <key name>
vconftool user_list
vconftool user_switch <user a.k.a subsession_id>
vconftool user_current
```

## DESCRIPTION

**vconftool** is a compatibility wrapper tool that provides vconf-style APIs for managing configuration data stored in Buxton2 backend. It allows users to set, get, and unset key-value pairs, as well as manage user sessions.

The tool maps vconf-style key names to Buxton2 layers:
- Keys starting with `user_memory` → user_memory layer
- Keys starting with `user` → user layer
- Keys starting with `memory` → memory layer (or memory_prd if key ends with `_prd`)
- All other keys → system layer (or system_prd if key ends with `_prd`)

## OPTIONS

### set-options

| Option | Description |
|--------|-------------|
| `-t`, `--type=int\|bool\|double\|string` | Type of key. Required for set operation. |
| `-u`, `--uid=UID` | User ID (for compatibility, ignored). |
| `-g`, `--gid=GID` | Group ID (for compatibility, ignored). |
| `-i`, `--install` | Install memory type key. |
| `-s`, `--smack=LABEL` | Set a SMACK label for the key. |
| `-f`, `--force` | Overwrite existing key by force. |

### get-options

| Option | Description |
|--------|-------------|
| `-r`, `--recursive` | Retrieve all keys having the given prefix (disabled in Tizen 3.0). |

### Other Options

| Option | Description |
|--------|-------------|
| `-d`, `--debug` | Enable debug output. |
| `-v`, `--verbose` | Verbose mode (ignored). |
| `-q`, `--quiet` | Quiet mode (ignored). |

## COMMANDS

| Command | Description |
|---------|-------------|
| `set` | Set a key-value pair. Creates the key if it doesn't exist. |
| `get` | Get the value of a key. |
| `unset` | Delete a key. |
| `user_list` | Print all users except for default user. |
| `user_switch` | Switch to specified user (requires buxton2 daemon). Empty string switches to default user. |
| `user_current` | Print the current user (requires buxton2 daemon). |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments, key not found, or operation failed. |

## EXAMPLES

### Set a string key

```bash
$ vconftool set -t string db/testapp/key1 "This is test"
```

### Set an integer key with memory type

```bash
$ vconftool set -t int -i memory/testapp/status -1
```

### Set a key with SMACK label

```bash
$ vconftool set -t string -s "User::App::testapp" db/testapp/key2 "value"
```

### Force overwrite an existing key

```bash
$ vconftool set -t string -f db/testapp/key1 "new value"
```

### Get a key value

```bash
$ vconftool get db/testapp/key1
db/testapp/key1, value = This is test (string)
```

### Unset a key

```bash
$ vconftool unset db/testapp/key1
```

### List all users

```bash
$ vconftool user_list
```

### Switch user

```bash
$ vconftool user_switch guest
```

### Get current user

```bash
$ vconftool user_current
```

## NOTES

- This tool is a wrapper around `buxton2ctl` for vconf API compatibility.
- When buxton2 daemon is not running, the tool operates in direct mode.
- The recursive option (`-r`) is disabled in Tizen 3.0.
- Keys are automatically mapped to appropriate Buxton2 layers based on naming conventions.

## SEE ALSO

**buxton2ctl**(1)

## PACKAGE

`vconf-compat`
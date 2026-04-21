# NAME

preference_tool - Tizen application preference management tool

# SYNOPSIS

```bash
preference_tool <command> [arguments]
```

# DESCRIPTION

`preference_tool` is a command-line tool for managing Tizen application preferences. It allows setting, getting, and listing preference values for applications.

# COMMANDS

| Command | Arguments | Description |
|---------|-----------|-------------|
| `set` | `<pkgid> <key_name> <type> <value>` | Set a preference value |
| `get` | `<pkgid> [<key>]` | Get preference value(s) for a package |
| `get_all` | - | Get all preference values |

## Data Types

The following types are supported for the `set` command:

| Type | Description |
|------|-------------|
| `int` | Integer value |
| `double` | Double precision floating point |
| `string` | String value |
| `bool` | Boolean value (true/false) |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Outputs preference values or error messages

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Operation executed successfully |
| nonzero | Error - Parameter invalid, permission denied, or process failure |

# FILES

- `/opt/dbspace/` - Preference database storage
- `/opt/usr/apps/` - Application data directories

# EXAMPLES

## Set a preference value

```bash
$ preference_tool set org.tizen.helloworld key string value
```

## Get all preferences for a package

```bash
$ preference_tool get org.tizen.helloworld
```

## Get a specific preference value

```bash
$ preference_tool get org.tizen.helloworld key
```

## Get all preferences on the system

```bash
$ preference_tool get_all
```

# SEE ALSO

preference(3), dlogutil(1)

# PACKAGE

preference
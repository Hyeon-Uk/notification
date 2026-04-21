# system-info-tool

## NAME

system-info-tool - Tizen system information query and management tool

## SYNOPSIS

```
system-info-tool [OPTION...]
```

## DESCRIPTION

system-info-tool is a command-line tool for querying and managing system information (features, properties) on the Tizen platform. It can query or set key-value pairs from the platform database, HAL database, and user-defined database.

**Database Priority:**
1. User-defined - Highest priority
2. Platform (platform defaults)
3. HAL (Hardware Abstraction Layer)

For boolean types, Platform and HAL values are combined with AND operation.

## OPTIONS

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-g KEY` | `--get KEY` | Query the value of specified KEY |
| `-v` | `--verbose` | Show detailed information (use with -g option) |
| `-l` | `--list-all` | List all system information database entries |
| `-s KEY TYPE VALUE` | `--set KEY TYPE VALUE` | Add or modify a user-defined KEY |
| `-c KEY` | `--clear KEY` | Delete a user-defined KEY (system keys cannot be deleted) |
| `-C` | `--clear-all` | Delete all user-defined keys |
| `-h` | `--help` | Show help |

### Supported Types (TYPE)

| Type | Description | Example Values |
|------|-------------|----------------|
| `bool` | Boolean | `true`, `false`, `1`, `0` |
| `int` | Integer | `42`, `-10` |
| `double` | Floating-point | `3.14`, `-0.5` |
| `string` | String | `Tizen`, `Samsung` |

## EXAMPLES

Query value of a specific key:
```
$ system-info-tool -g tizen.org/feature/display
type=bool, value=1
```

Query with detailed information:
```
$ system-info-tool -g tizen.org/feature/battery -v
   Platform Key (A): type=bool, value=1
        HAL Key (B): type=bool, value=0
  User-defined Key (C): not found
Result (A && B): type=bool, value=0
```

List all keys:
```
$ system-info-tool -l
```

Set a user-defined key (boolean):
```
$ system-info-tool -s tizen.org/feature/display bool true
```

Set a user-defined key (string):
```
$ system-info-tool -s tizen.org/system/manufacturer string Tizen
```

Set a user-defined key (integer):
```
$ system-info-tool -s tizen.org/system/build.version int 12345
```

Delete a user-defined key:
```
$ system-info-tool -c tizen.org/feature/display
```

Delete all user-defined keys:
```
$ system-info-tool -C
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred (key not found, invalid argument, etc.) |

## NOTES

- User-defined keys have higher priority than system keys
- Read-only system keys cannot be deleted
- The same key may exist in multiple types (type selection required when deleting)
- The `http://` prefix is automatically removed from keys

## SEE ALSO

- system_info(3)
- model-config-cli(1)
- Tizen System Information API

## PACKAGE

system-info-tools
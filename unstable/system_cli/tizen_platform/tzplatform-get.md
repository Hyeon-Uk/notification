# tzplatform-get

## NAME

tzplatform-get - Tizen platform configuration variable query tool

## SYNOPSIS

```
tzplatform-get [options...] [keys...]
```

## DESCRIPTION

tzplatform-get is a command-line tool for querying configuration variables (key-value pairs) of the Tizen platform. It allows users to view various paths and configuration values used in platform configuration.

## OPTIONS

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-a` | `--all` | Print all keys (must be used without key arguments) |
| `-n` | `--not` | Print keys except the specified ones |
| `-e` | `--export` | Add 'export' prefix to output (for shell environment variable setup) |
| `-l` | `--list` | Print key names only (without values) |
| `-s` | `--space` | Separate output with space instead of newline |
| `-q` | `--query` | Silently check if specified variable exists (returns result via exit code) |
| `-c` | `--continue` | Continue processing even on errors |
| `-u id` | `--user id` | Specify user ID (name or number) to query in that user's context |

## EXAMPLES

Query value of a specific key:
```
$ tzplatform-get TZ_USER_APP
/home/usr/apps
```

Query all keys and values:
```
$ tzplatform-get --all
```

Print in environment variable format:
```
$ tzplatform-get -e TZ_USER_APP
export TZ_USER_APP=/home/usr/apps
```

List key names only:
```
$ tzplatform-get -l --all
```

Query all keys except specified ones:
```
$ tzplatform-get --all --not TZ_SYS_RO TZ_SYS_RW
```

Check if variable exists (via exit code):
```
$ tzplatform-get -q TZ_USER_APP
$ echo $?
0
```

Print with space separation:
```
$ tzplatform-get -s TZ_USER_APP TZ_SYS_APP
/home/usr/apps /usr/apps
```

Query in specific user context:
```
$ tzplatform-get -u 5001 TZ_USER_APP
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred (key not found, etc.) |

## SEE ALSO

- tzplatform-config(5)
- Tizen Platform Configuration Guide

## PACKAGE

tizen-platform-config-tools
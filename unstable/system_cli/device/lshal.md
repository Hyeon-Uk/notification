# lshal - List platform HAL and backend information

## NAME

**lshal** - List platform HAL (Hardware Abstraction Layer) and associated backend information.

## SYNOPSIS

```bash
lshal [OPTION]...
```

## DESCRIPTION

**lshal** is a command-line tool to list platform HAL (Hardware Abstraction Layer) modules and their associated backend information on Tizen systems. It provides visibility into the HAL configuration and helps debug hardware abstraction issues.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message. |
| `-r`, `--reset` | Clear HAL information cache. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### List HAL information

```bash
$ lshal
```

### Clear HAL cache and refresh

```bash
$ lshal -r
```

## NOTES

- HAL (Hardware Abstraction Layer) provides a consistent interface between hardware and higher-level software.
- The tool displays platform-specific HAL modules and their backend implementations.
- Use `-r` option to refresh cached information when HAL configuration changes.

## SEE ALSO

**hal-api-common**(7)

## PACKAGE

`hal-api-common`
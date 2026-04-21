# dmstats - Device-mapper statistics management

## NAME

**dmstats** - Manage device-mapper I/O statistics regions and reports.

## SYNOPSIS

```bash
dmstats [options] <command> [command_args]
```

## DESCRIPTION

**dmstats** is a utility to manage device-mapper I/O statistics. It allows users to create, delete, and report statistics regions for block devices, providing detailed I/O performance metrics.

## OPTIONS

### General Options

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message. |
| `-v`, `--verbose` | Verbose mode. Repeat for more verbosity. |
| `--areas <nr_areas>` | Number of statistics areas. |
| `--areasize <size>` | Size of each statistics area. |
| `--userdata <data>` | User data string for region. |
| `--clear` | Clear statistics counters. |
| `--count <count>` | Number of report iterations. |
| `--interval <seconds>` | Report interval. |
| `-o <fields>` | Specify output fields. |
| `-O`, `--sort <sort_fields>` | Sort output by fields. |
| `--programid <id>` | Program identifier. |
| `--start <start>` | Starting offset. |
| `--length <length>` | Length of region. |
| `--segments` | Create regions for each segment. |
| `--units <units>` | Output units. |

## COMMANDS

| Command | Description |
|---------|-------------|
| `help` | Show help. |
| `clear` | Clear statistics for regions. |
| `create` | Create statistics regions. |
| `delete` | Delete statistics regions. |
| `group` | Group statistics regions. |
| `list` | List statistics regions. |
| `print` | Print statistics counters. |
| `report` | Report I/O statistics. |
| `ungroup` | Ungroup statistics regions. |
| `update_filemap` | Update filemap statistics. |
| `version` | Show version. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### List all statistics regions

```bash
$ dmstats list
```

### Create statistics region on device

```bash
$ dmstats create /dev/mapper/mydevice
```

### Create with specific areas

```bash
$ dmstats create --areas 10 /dev/mapper/mydevice
```

### Create with area size

```bash
$ dmstats create --areasize 1M /dev/mapper/mydevice
```

### Create with program ID

```bash
$ dmstats create --programid myapp /dev/mapper/mydevice
```

### Create filemap statistics

```bash
$ dmstats create --filemap /path/to/file
```

### Report statistics

```bash
$ dmstats report
$ dmstats report /dev/mapper/mydevice
```

### Print statistics counters

```bash
$ dmstats print
```

### Clear statistics

```bash
$ dmstats clear --alldevices
```

### Delete statistics regions

```bash
$ dmstats delete --alldevices
$ dmstats delete --programid myapp /dev/mapper/mydevice
```

### Group regions

```bash
$ dmstats group --regions 1,2,3
```

### Continuous monitoring

```bash
$ dmstats report --interval 1 --count 10
```

## NOTES

- Device-mapper statistics provide fine-grained I/O metrics.
- Statistics can be collected per-region with configurable boundaries.
- Filemap mode allows tracking I/O for specific files.
- Use program IDs to identify statistics from different applications.
- Requires root privileges for most operations.

## SEE ALSO

**dmsetup**(8), **dmsetup stats**(8)

## PACKAGE

`device-mapper`
# dmsetup - Device mapper management tool

## NAME

**dmsetup** - Low level logical volume management tool for device-mapper.

## SYNOPSIS

```bash
dmsetup [options] <command> [command_args]
```

## DESCRIPTION

**dmsetup** is a low level logical volume management tool for the device-mapper driver. Device-mapper is a kernel framework for volume management. It provides a generic way to create virtual layers of block devices that can do various things like striping, mirroring, snapshotting, and more.

## OPTIONS

### General Options

| Option | Description |
|--------|-------------|
| `--version` | Print version information. |
| `-h`, `--help` | Show help message. |
| `-v`, `--verbose` | Verbose mode. Repeat for more verbosity. |
| `-f`, `--force` | Force operation. |
| `--checks` | Enable additional checks. |
| `--manglename {none\|hex\|auto}` | Set name mangling mode. |
| `-r`, `--readonly` | Operate in read-only mode. |
| `--noopencount` | Disable open count check. |
| `--noflush` | Don't flush I/O. |
| `--nolockfs` | Don't lock filesystem. |
| `--inactive` | Operate on inactive table. |
| `-y`, `--yes` | Answer yes to all prompts. |
| `--readahead {[+]<sectors>\|auto\|none}` | Set read ahead. |
| `--retry` | Retry on failure. |

### Output Options

| Option | Description |
|--------|-------------|
| `-c`, `-C`, `--columns` | Column-based output. |
| `-o <fields>` | Specify output fields. |
| `-O`, `--sort <sort_fields>` | Sort output by fields. |
| `-S`, `--select <selection>` | Select devices by criteria. |
| `--nameprefixes` | Prefix output fields. |
| `--noheadings` | Don't print headings. |
| `--separator <separator>` | Set output separator. |

## COMMANDS

### Device Management

| Command | Description |
|---------|-------------|
| `create <dev_name>` | Create a device. |
| `remove <device>` | Remove a device. |
| `remove_all` | Remove all devices. |
| `rename <device> <new_name>` | Rename a device. |
| `clear <device>` | Clear device table. |
| `suspend <device>` | Suspend a device. |
| `resume <device>` | Resume a device. |
| `load <device> [<table>]` | Load table into device. |
| `reload <device> [<table>]` | Reload device table. |
| `wipe_table <device>` | Wipe device table. |

### Device Information

| Command | Description |
|---------|-------------|
| `ls` | List devices. |
| `info [<device>]` | Show device information. |
| `status [<device>]` | Show device status. |
| `table [<device>]` | Show device table. |
| `deps [<device>]` | Show device dependencies. |
| `stats <command>` | Manage device statistics. |
| `targets` | List available targets. |
| `version` | Show version. |
| `target-version [<target>]` | Show target version. |

### Device Node Management

| Command | Description |
|---------|-------------|
| `mknodes [<device>]` | Create device nodes. |
| `mangle [<device>]` | Fix device names. |
| `setgeometry <device> <cyl> <head> <sect> <start>` | Set device geometry. |
| `splitname <device>` | Split device name. |

### Event Management

| Command | Description |
|---------|-------------|
| `wait <device> [<event_nr>]` | Wait for event. |
| `message <device> <sector> <message>` | Send message to device. |
| `measure [<device>]` | Measure IMA. |

### Udev Management

| Command | Description |
|---------|-------------|
| `udevcreatecookie` | Create udev cookie. |
| `udevreleasecookie [<cookie>]` | Release udev cookie. |
| `udevflags <cookie>` | Get udev flags. |
| `udevcomplete <cookie>` | Mark udev complete. |
| `udevcomplete_all [<age_in_minutes>]` | Complete all udev. |
| `udevcookies` | List udev cookies. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or operation failed. |
| `2` | Device not found. |

## EXAMPLES

### List all device-mapper devices

```bash
$ dmsetup ls
```

### List devices as tree

```bash
$ dmsetup ls --tree
```

### Show device information

```bash
$ dmsetup info
$ dmsetup info mydevice
```

### Show device table

```bash
$ dmsetup table
```

### Show device status

```bash
$ dmsetup status
```

### Create a device

```bash
$ dmsetup create mydevice --table "0 100 linear /dev/sda1 0"
```

### Remove a device

```bash
$ dmsetup remove mydevice
```

### Suspend and resume device

```bash
$ dmsetup suspend mydevice
$ dmsetup resume mydevice
```

### Create device with concised spec

```bash
$ dmsetup create --concise "dev1,,,,0 100 linear 253:1 0"
```

### Show available targets

```bash
$ dmsetup targets
```

## NOTES

- Device-mapper is a kernel framework for creating virtual block devices.
- Requires root privileges for most operations.
- Device names can be specified by name, UUID (`-u`), or major:minor (`-j -m`).
- LVM2 and other volume managers use device-mapper internally.

## SEE ALSO

**dmsetup**(8), **dmstats**(8), **lvm**(8)

## PACKAGE

`device-mapper`
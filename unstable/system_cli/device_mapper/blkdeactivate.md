# blkdeactivate - Utility to deactivate block devices

## NAME

**blkdeactivate** - Utility to deactivate block device tree.

## SYNOPSIS

```bash
blkdeactivate [options] [device...]
```

## DESCRIPTION

**blkdeactivate** is a utility to deactivate block device trees. If devices are specified, it deactivates only the supplied devices and their holders. This tool handles various types of block devices including device-mapper, LVM, multipath, MD RAID, and VDO.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-e`, `--errors` | Show errors reported from tools. |
| `-h`, `--help` | Show help message. |
| `-u`, `--umount` | Unmount the device if mounted. |
| `-v`, `--verbose` | Verbose mode (also implies -e). |
| `-d`, `--dmoptions DM_OPTIONS` | Comma separated DM specific options. |
| `-l`, `--lvmoptions LVM_OPTIONS` | Comma separated LVM specific options. |
| `-m`, `--mpathoptions MPATH_OPTIONS` | Comma separated DM-multipath specific options. |
| `-r`, `--mdraidoptions MDRAID_OPTIONS` | Comma separated MD RAID specific options. |
| `-o`, `--vdooptions VDO_OPTIONS` | Comma separated VDO specific options. |

## DEVICE SPECIFIC OPTIONS

### DM_OPTIONS

| Option | Description |
|--------|-------------|
| `retry` | Retry removal several times in case of failure. |
| `force` | Force device removal. |

### LVM_OPTIONS

| Option | Description |
|--------|-------------|
| `retry` | Retry removal several times in case of failure. |
| `wholevg` | Deactivate the whole VG when processing an LV. |

### MDRAID_OPTIONS

| Option | Description |
|--------|-------------|
| `wait` | Wait for resync, recovery or reshape to complete first. |

### MPATH_OPTIONS

| Option | Description |
|--------|-------------|
| `disablequeueing` | Disable queueing on all DM-multipath devices first. |

### VDO_OPTIONS

| Option | Description |
|--------|-------------|
| `configfile=file` | Use specified VDO configuration file. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. All devices deactivated. |
| `1` | Error. Some devices could not be deactivated. |

## EXAMPLES

### Deactivate all block devices

```bash
$ blkdeactivate
```

### Deactivate specific device

```bash
$ blkdeactivate /dev/mapper/mydevice
```

### Deactivate with verbose output

```bash
$ blkdeactivate -v
```

### Deactivate with unmounting

```bash
$ blkdeactivate -u
```

### Deactivate with DM force option

```bash
$ blkdeactivate -d force
```

### Deactivate LVM with whole VG

```bash
$ blkdeactivate -l wholevg
```

### Combine multiple options

```bash
$ blkdeactivate -v -u -d retry,force
```

## NOTES

- Requires root privileges to execute.
- Deactivates devices in proper order to handle dependencies.
- Automatically handles LVM, DM, multipath, and MD RAID devices.
- Use `-v` to see detailed progress and any errors.

## SEE ALSO

**dmsetup**(8), **lvm**(8), **multipath**(8), **mdadm**(8)

## PACKAGE

`device-mapper`
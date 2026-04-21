# mkfs.btrfs - Create a Btrfs filesystem

## NAME

**mkfs.btrfs** - Create a Btrfs filesystem on a device.

## SYNOPSIS

```bash
mkfs.btrfs [OPTIONS] <device> [<device>...]
```

## DESCRIPTION

**mkfs.btrfs** creates a Btrfs filesystem on one or more devices. Btrfs is a copy-on-write (COW) filesystem for Linux that supports advanced features like snapshots, subvolumes, RAID, and more.

The device is usually a block device (e.g., `/dev/sda1`) but can also be a file or a disk image.

## OPTIONS

### Basic Options

| Option | Description |
|--------|-------------|
| `-b <size>`, `--byte-count <size>` | Specify filesystem size. |
| `-d <profile>`, `--data <profile>` | Data profile: raid0, raid1, raid1c3, raid1c4, raid5, raid6, raid10, single, dup. |
| `-f`, `--force` | Force overwrite. |
| `-L <label>`, `--label <label>` | Set filesystem label. |
| `-m <profile>`, `--metadata <profile>` | Metadata profile: raid0, raid1, raid1c3, raid1c4, raid5, raid6, raid10, single, dup. |
| `-M`, `--mixed` | Mix data and metadata. |
| `-n <size>`, `--nodesize <size>` | Set nodesize (default: 16384). |
| `-s <size>`, `--sectorsize <size>` | Set sectorsize (default: 4096). |
| `-r <rootdir>`, `--rootdir <rootdir>` | Populate filesystem from directory. |
| `--shrink` | Shrink filesystem to minimal size (with -r). |
| `-K`, `--nodiscard` | Do not perform TRIM on device. |
| `-O <features>`, `--features <features>` | Enable features. |
| `-O --list-features` | List available features. |
| `-U <uuid>`, `--uuid <uuid>` | Specify UUID. |
| `--device-uuid <uuid>` | Specify device UUID. |
| `--device-sector-size <size>` | Specify device sector size. |

### RAID Profile Options

| Profile | Description |
|---------|-------------|
| `single` | No redundancy (default for data on single device). |
| `dup` | Duplicate on same device (default for metadata on single device). |
| `raid0` | Striping across devices. |
| `raid1` | Mirroring across 2 devices. |
| `raid1c3` | Mirroring across 3 devices. |
| `raid1c4` | Mirroring across 4 devices. |
| `raid10` | Striping + mirroring. |
| `raid5` | Striping with parity. |
| `raid6` | Striping with double parity. |

### Feature Options

| Feature | Description |
|---------|-------------|
| `mixed-bg` | Mixed block groups. |
| `extref` | Extended inode references. |
| `raid56` | RAID5/RAID6 support. |
| `skinny-metadata` | Skinny metadata. |
| `no-holes` | No holes representation. |
| `free-space-tree` | Free space tree. |
| `uuid-tree` | UUID tree. |
| `raid-stripe-tree` | RAID stripe tree. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Create filesystem on single device

```bash
$ mkfs.btrfs /dev/sda1
btrfs-progs v5.16
See http://btrfs.wiki.kernel.org for more information.

Label:              (null)
UUID:               12345678-1234-1234-1234-123456789012
Node size:          16384
Sector size:        4096
Filesystem size:    100.00GiB
Block group profiles:
  Data:             single            8.00MiB
  Metadata:         DUP             256.00MiB
  System:           DUP               8.00MiB
SSD detected:       no
Incompat features:  extref, skinny-metadata, no-holes
Checksum:           crc32c
Number of devices:  1
Devices:
   ID        SIZE  PATH
    1   100.00GiB  /dev/sda1
```

### Create filesystem with label

```bash
$ mkfs.btrfs -L mydata /dev/sda1
```

### Create RAID1 filesystem on two devices

```bash
$ mkfs.btrfs -d raid1 -m raid1 /dev/sda1 /dev/sdb1
```

### Create RAID5 filesystem on three devices

```bash
$ mkfs.btrfs -d raid5 -m raid5 /dev/sda1 /dev/sdb1 /dev/sdc1
```

### Create RAID10 filesystem on four devices

```bash
$ mkfs.btrfs -d raid10 -m raid10 /dev/sda1 /dev/sdb1 /dev/sdc1 /dev/sdd1
```

### Force overwrite existing filesystem

```bash
$ mkfs.btrfs -f /dev/sda1
```

### Create filesystem from directory

```bash
$ mkfs.btrfs -r /mydata /dev/sda1
```

### Create filesystem with specific nodesize

```bash
$ mkfs.btrfs -n 32K /dev/sda1
```

### List available features

```bash
$ mkfs.btrfs -O --list-features
Filesystem features:
  mixed-bg
  extref
  raid56
  skinny-metadata
  no-holes
  free-space-tree
  uuid-tree
  raid-stripe-tree
```

## NOTES

- Minimum device size is 114MiB (with mixed bg) or 256MiB.
- Default nodesize is 16KiB.
- Default sectorsize is 4KiB.
- For single device, data uses `single` profile and metadata uses `dup`.
- For multiple devices, default data profile is `single` and metadata is `raid1`.
- Use `-f` to force overwrite an existing filesystem.
- TRIM is performed by default on SSDs unless `-K` is specified.

## SEE ALSO

**btrfs**(8), **btrfsck**(8), **fsck.btrfs**(8)

## PACKAGE

`btrfs-progs`
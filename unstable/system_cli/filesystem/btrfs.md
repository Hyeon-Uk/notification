# btrfs - Btrfs filesystem management tool

## NAME

**btrfs** - Manage Btrfs filesystems (subvolume, device, balance, scrub, etc.)

## SYNOPSIS

```bash
btrfs [OPTIONS] <COMMAND> [ARGS]
```

## DESCRIPTION

**btrfs** is the main administration tool for Btrfs filesystems. Btrfs is a copy-on-write (COW) filesystem for Linux aimed at implementing advanced features while focusing on fault tolerance, repair and easy administration.

The tool provides various commands organized into groups for managing subvolumes, devices, balance operations, scrub, and more.

## COMMANDS

### Subvolume Commands

| Command | Description |
|---------|-------------|
| `subvolume create <path>` | Create a subvolume. |
| `subvolume delete <path>` | Delete a subvolume. |
| `subvolume list <path>` | List subvolumes. |
| `subvolume snapshot <src> <dst>` | Create a snapshot. |
| `subvolume get-default <path>` | Get default subvolume. |
| `subvolume set-default <id> <path>` | Set default subvolume. |
| `subvolume show <path>` | Show subvolume info. |

### Filesystem Commands

| Command | Description |
|---------|-------------|
| `filesystem df <path>` | Show filesystem space usage. |
| `filesystem du [options] <path>` | Disk usage calculator. |
| `filesystem show [device]` | Show filesystem info. |
| `filesystem sync <path>` | Force sync. |
| `filesystem defrag [options] <path>` | Defragment files. |
| `filesystem resize [size] <path>` | Resize filesystem. |
| `filesystem label <device> [label]` | Get/set label. |
| `filesystem usage <path>` | Show detailed usage. |

### Device Commands

| Command | Description |
|---------|-------------|
| `device add <device> <path>` | Add device to filesystem. |
| `device delete <device> <path>` | Remove device from filesystem. |
| `device remove <device> <path>` | Alias for delete. |
| `device scan [<device>]` | Scan for devices. |
| `device ready <device>` | Check device ready. |
| `device stats <path>` | Show device error stats. |
| `device usage <path>` | Show device usage. |

### Balance Commands

| Command | Description |
|---------|-------------|
| `balance start [options] <path>` | Start balance. |
| `balance pause <path>` | Pause balance. |
| `balance cancel <path>` | Cancel balance. |
| `balance resume <path>` | Resume balance. |
| `balance status <path>` | Show balance status. |

### Scrub Commands

| Command | Description |
|---------|-------------|
| `scrub start [options] <path>` | Start scrub. |
| `scrub cancel <path>` | Cancel scrub. |
| `scrub resume <path>` | Resume scrub. |
| `scrub status <path>` | Show scrub status. |

### Rescue Commands

| Command | Description |
|---------|-------------|
| `rescue chunk-recover <device>` | Recover chunk tree. |
| `rescue super-recover <device>` | Recover superblock. |
| `rescue zero-log <device>` | Clear log tree. |
| `rescue fix-device-size <device>` | Fix device size. |

### Inspect-Internal Commands

| Command | Description |
|---------|-------------|
| `inspect-internal dump-super [device]` | Dump superblock. |
| `inspect-internal dump-tree [device]` | Dump btree. |
| `inspect-internal tree-stats [device]` | Show tree stats. |

### Property Commands

| Command | Description |
|---------|-------------|
| `property get <path>` | Get property. |
| `property set <path> <name> <value>` | Set property. |
| `property list <path>` | List properties. |

### Quota Commands

| Command | Description |
|---------|-------------|
| `quota enable <path>` | Enable quota. |
| `quota disable <path>` | Disable quota. |
| `quota rescan <path>` | Rescan quota. |

### Qgroup Commands

| Command | Description |
|---------|-------------|
| `qgroup assign <src> <dst> <path>` | Assign qgroup. |
| `qgroup remove <src> <dst> <path>` | Remove qgroup. |
| `qgroup create <qgroupid> <path>` | Create qgroup. |
| `qgroup destroy <qgroupid> <path>` | Destroy qgroup. |
| `qgroup show <path>` | Show qgroups. |
| `qgroup limit <size> <qgroupid> <path>` | Set qgroup limit. |

### Replace Commands

| Command | Description |
|---------|-------------|
| `replace start <srcdev> <tgtdev> <path>` | Start device replace. |
| `replace cancel <path>` | Cancel replace. |
| `replace status <path>` | Show replace status. |

### Send/Receive Commands

| Command | Description |
|---------|-------------|
| `send [options] <subvol>` | Send subvolume. |
| `receive [options] <path>` | Receive subvolume. |

## OPTIONS

### Global Options

| Option | Description |
|--------|-------------|
| `--help` | Show help. |
| `--version` | Show version. |
| `-v`, `--verbose` | Verbose output. |
| `-q`, `--quiet` | Suppress output. |

### Subvolume Options

| Option | Description |
|--------|-------------|
| `-r`, `--read-only` | Create read-only snapshot. |
| `-p <id>`, `--parent-id <id>` | Set parent subvolume ID. |
| `-i <id>`, `--subvolid <id>` | Specify subvolume ID. |

### Balance Options

| Option | Description |
|--------|-------------|
| `-d[<filters>]` | Balance data chunks. |
| `-m[<filters>]` | Balance metadata chunks. |
| `-s[<filters>]` | Balance system chunks. |
| `-v`, `--verbose` | Verbose output. |
| `-f`, `--force` | Force operation. |

### Scrub Options

| Option | Description |
|--------|-------------|
| `-B` | Do not background. |
| `-d` | Stats per device. |
| `-q` | Quiet. |
| `-r` | Read only mode. |
| `-R` | Raw output. |
| `-c <ioprio>` | IO priority class. |
| `-n <ioprio>` | IO priority level. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |
| `2` | Invalid command or option. |

## EXAMPLES

### Create a subvolume

```bash
$ btrfs subvolume create /mnt/btrfs/subvol1
Create subvolume '/mnt/btrfs/subvol1'
```

### Create a snapshot

```bash
$ btrfs subvolume snapshot /mnt/btrfs/subvol1 /mnt/btrfs/snap1
Create a snapshot of '/mnt/btrfs/subvol1' in '/mnt/btrfs/snap1'
```

### Create a read-only snapshot

```bash
$ btrfs subvolume snapshot -r /mnt/btrfs/subvol1 /mnt/btrfs/snap-ro
Create a readonly snapshot of '/mnt/btrfs/subvol1' in '/mnt/btrfs/snap-ro'
```

### List subvolumes

```bash
$ btrfs subvolume list /mnt/btrfs
ID 256 gen 100 top level 5 path subvol1
ID 257 gen 101 top level 5 path snap1
```

### Show filesystem info

```bash
$ btrfs filesystem show
Label: 'myfs'  uuid: 12345678-1234-1234-1234-123456789012
        Total devices 1 FS bytes used 1.00GiB
        devid    1 size 10.00GiB used 2.00GiB path /dev/sda1
```

### Check disk usage

```bash
$ btrfs filesystem df /mnt/btrfs
Data, single: total=1.00GiB, used=512.00MiB
System, DUP: total=8.00MiB, used=16.00KiB
Metadata, DUP: total=256.00MiB, used=128.00MiB
GlobalReserve, single: total=16.00MiB, used=0.00B
```

### Add a device

```bash
$ btrfs device add /dev/sdb1 /mnt/btrfs
```

### Start balance

```bash
$ btrfs balance start /mnt/btrfs
Done, had to relocate 0 out of 100 chunks
```

### Start scrub

```bash
$ btrfs scrub start /mnt/btrfs
scrub started on /mnt/btrfs, fsid 12345678-1234-1234-1234-123456789012
```

### Send subvolume (for backup)

```bash
$ btrfs send /mnt/btrfs/snap1 | btrfs receive /backup/btrfs/
At subvol /mnt/btrfs/snap1
At subvol snap1
```

## NOTES

- Btrfs uses copy-on-write (COW) technology.
- Subvolumes are like separate namespaces within a filesystem.
- Snapshots are space-efficient due to COW.
- Balance relocates data chunks across devices.
- Scrub verifies data and metadata integrity.
- Commands can be abbreviated as long as they remain unambiguous.

## SEE ALSO

**mkfs.btrfs**(8), **btrfsck**(8), **btrfstune**(8), **fsck.btrfs**(8)

## PACKAGE

`btrfs-progs`
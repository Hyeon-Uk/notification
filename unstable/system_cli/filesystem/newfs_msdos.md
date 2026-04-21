# newfs_msdos - Create MS-DOS/FAT filesystem

## NAME

**newfs_msdos** - Create an MS-DOS/FAT (File Allocation Table) filesystem.

## SYNOPSIS

```bash
newfs_msdos [OPTIONS] <device>
```

## DESCRIPTION

**newfs_msdos** creates an MS-DOS/FAT filesystem on the specified device. This includes FAT12, FAT16, and FAT32 filesystems, commonly used for removable media and embedded systems.

## OPTIONS

### Basic Options

| Option | Description |
|--------|-------------|
| `-N <num>`, `--num-sectors <num>` | Number of sectors. |
| `-B <file>`, `--boot-block <file>` | Use specified boot block. |
| `-F <type>`, `--fat-type <type>` | FAT type: 12, 16, or 32. |
| `-I <vol-id>`, `--volume-id <vol-id>` | Volume ID. |
| `-L <label>`, `--volume-label <label>` | Volume label. |
| `-O <oem>`, `--oem-string <oem>` | OEM string. |
| `-S <size>`, `--sector-size <size>` | Sector size (512, 1024, 2048, 4096). |
| `-s <sectors>`, `--sectors-per-cluster <sectors>` | Sectors per cluster. |

### Geometry Options

| Option | Description |
|--------|-------------|
| `-b <sectors>`, `--block-size <sectors>` | Block size. |
| `-c <clusters>`, `--cluster-size <clusters>` | Cluster size. |
| `-f <count>`, `--fats <count>` | Number of FATs (default: 2). |
| `-h <heads>`, `--heads <heads>` | Number of heads. |
| `-k <sector>`, `--backup-sector <sector>` | Backup boot sector. |
| `-m <media>`, `--media-descriptor <media>` | Media descriptor. |
| `-n <sectors>`, `--sectors-per-track <sectors>` | Sectors per track. |
| `-r <entries>`, `--root-dir-entries <entries>` | Root directory entries. |
| `-u <sectors>`, `--sectors-per-fat <sectors>` | Sectors per FAT. |

### Other Options

| Option | Description |
|--------|-------------|
| `-a`, `--all` | Use all device. |
| `-f`, `--force` | Force creation. |
| `-R <sectors>`, `--reserved-sectors <sectors>` | Reserved sectors. |
| `-v`, `--verbose` | Verbose output. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Create FAT32 filesystem

```bash
$ newfs_msdos -F 32 /dev/sda1
```

### Create with label

```bash
$ newfs_msdos -F 32 -L MYDISK /dev/sda1
```

### Create FAT16 filesystem

```bash
$ newfs_msdos -F 16 /dev/sda1
```

### Create with specific cluster size

```bash
$ newfs_msdos -F 32 -s 64 /dev/sda1
```

### Create with specific sector size

```bash
$ newfs_msdos -F 32 -S 4096 /dev/sda1
```

## NOTES

- FAT12 is for volumes < 16MB.
- FAT16 is for volumes 16MB - 2GB.
- FAT32 is for volumes 32MB - 2TB.
- Default FAT type is automatically determined by volume size.
- exFAT should be used for volumes > 32GB on modern systems.

## SEE ALSO

**mkfs.fat**(8), **fsck.fat**(8), **mkfs.vfat**(8)

## PACKAGE

`dosfstools`
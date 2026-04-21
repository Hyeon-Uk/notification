# mkfs.f2fs - Create an F2FS filesystem

## NAME

**mkfs.f2fs** - Create an F2FS (Flash-Friendly File System) filesystem.

## SYNOPSIS

```bash
mkfs.f2fs [OPTIONS] <device>
```

## DESCRIPTION

**mkfs.f2fs** creates an F2FS filesystem on the specified device. F2FS (Flash-Friendly File System) is a Linux filesystem designed for NAND flash memory-based storage devices, such as SSDs, eMMC, and SD cards.

F2FS is optimized for flash storage characteristics and provides features like TRIM support, cleaning operations, and adaptive logging.

## OPTIONS

### Basic Options

| Option | Description |
|--------|-------------|
| `-l <label>`, `--label <label>` | Set filesystem label. |
| `-a <mode>`, `--heap <mode>` | Allocation mode (0: auto, 1: heap). |
| `-o <offset>`, `--offset <offset>` | Start offset (in sectors). |
| `-s <size>`, `--size <size>` | Size (in sectors). |
| `-t <sectors>`, `--trim <sectors>` | Set TRIM sectors. |
| `-z <zones>`, `--zones <zones>` | Set number of zones. |
| `-f`, `--force` | Force overwrite. |
| `-m`, `--mode <mode>` | Set mode: 0 for block-aligned, 1 for sector-aligned. |
| `-O <features>`, `--feature <features>` | Enable features. |
| `-C <encoding>`, `--casefold <encoding>` | Enable casefolding with specified encoding. |
| `-d <slab-size>`, `--dentry-level <slab-size>` | Set dentry slab size. |
| `-i <slab-size>`, `--inode-level <slab-size>` | Set inode slab size. |
| `-w`, `--writable` | Create writable checkpoint. |

### Feature Options

| Option | Description |
|--------|-------------|
| `-O extra_attr` | Enable extra attributes. |
| `-O project_quota` | Enable project quota. |
| `-O inode_checksum` | Enable inode checksum. |
| `-O flexible_inline_xattr` | Enable flexible inline xattr. |
| `-O quota` | Enable quota. |
| `-O inode_crtime` | Enable inode creation time. |
| `-O lost_found` | Enable lost+found directory. |
| `-O verity` | Enable fs-verity. |
| `-O casefold` | Enable casefolding. |
| `-O compression` | Enable compression. |
| `-O encrypt` | Enable encryption. |

### Compression Options

| Option | Description |
|--------|-------------|
| `-c <algorithm>`, `--compress <algorithm>` | Compression algorithm: lzo, lz4, zstd, lzo-rle. |
| `-r <log-size>`, `--compress-log-size <log-size>` | Set compression cluster log size. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Create F2FS filesystem

```bash
$ mkfs.f2fs /dev/sda1

        F2FS-tools: mkfs.f2fs Ver: 1.15.0 (2022-04-04)

Info: [/dev/sda1] Disk model: SSD
Info: [/dev/sda1] F2FS_FEATURE: encrypt, verity, quota, project_quota, extra_attr, flexible_inline_xattr, inode_checksum, inode_crtime, compression
Info: Segments per section = 1
Info: Sections per zone = 1
Info: sector size = 512
Info: total sectors = 209715200 (102400 MB)
Info: zone aligned segment0 blkaddr: 512
Info: format version with = 2
Info: [/dev/sda1] Discarding device
Info: This device doesn't support BLKSECDISCARD
Info: Discarded 102400 MB
Info: [/dev/sda1] Discard: Success
Info: Format successful!
```

### Create filesystem with label

```bash
$ mkfs.f2fs -l mydata /dev/sda1
```

### Force overwrite existing filesystem

```bash
$ mkfs.f2fs -f /dev/sda1
```

### Create with encryption support

```bash
$ mkfs.f2fs -O encrypt /dev/sda1
```

### Create with compression support

```bash
$ mkfs.f2fs -O compression -c lz4 /dev/sda1
```

### Create with multiple features

```bash
$ mkfs.f2fs -O encrypt,verity,quota,compression /dev/sda1
```

## NOTES

- F2FS is optimized for NAND flash storage devices.
- Supports TRIM/DISCARD by default.
- Default sector size is 512 bytes.
- Minimum filesystem size is about 30MB.
- Compression requires kernel support (5.6+).
- Encryption requires kernel support (4.2+).

## SEE ALSO

**fsck.f2fs**(8), **dump.f2fs**(8), **resize.f2fs**(8), **f2fs_io**(8)

## PACKAGE

`f2fs-tools`
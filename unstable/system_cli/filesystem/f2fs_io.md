# f2fs_io - F2FS I/O control utility

## NAME

**f2fs_io** - Control and query F2FS filesystem I/O operations.

## SYNOPSIS

```bash
f2fs_io <command> [args] <file>
```

## DESCRIPTION

**f2fs_io** is a utility for controlling and querying various I/O operations and features of F2FS filesystems. It provides access to F2FS-specific ioctls and features.

## COMMANDS

### File Commands

| Command | Description |
|---------|-------------|
| `getflags <file>` | Get file flags. |
| `setflags <flags> <file>` | Set file flags. |
| `getversion <file>` | Get file version. |
| `setversion <version> <file>` | Set file version. |

### Compression Commands

| Command | Description |
|---------|-------------|
| `get_coption <file>` | Get compression options. |
| `set_coption <algorithm> <log_cluster_size> <file>` | Set compression options. |
| `start_compress <file>` | Start compression. |
| `release_compress <file>` | Release compressed blocks. |

### Pinning Commands

| Command | Description |
|---------|-------------|
| `pinfile <on\|off> <file>` | Pin/unpin file in memory. |
| `get_pinfile <file>` | Get pin file status. |

### Garbage Collection Commands

| Command | Description |
|---------|-------------|
| `gc_urgent <on\|off> <path>` | Enable/disable urgent GC. |
| `gc_idle <mode> <path>` | Set GC idle mode (0: off, 1: normal, 2: idle). |
| `get_gc_status <path>` | Get GC status. |

### Other Commands

| Command | Description |
|---------|-------------|
| `flush_device <device>` | Flush device cache. |
| `trim_device <device>` | Trim device. |
| `get_advise <file>` | Get F2FS advise. |
| `set_advise <advise> <file>` | Set F2FS advise. |
| `get_disk_layout <path>` | Get disk layout info. |
| `get_block_info <file>` | Get block allocation info. |

## FILE FLAGS

| Flag | Description |
|------|-------------|
| `append` | Append-only file. |
| `immutable` | Immutable file. |
| `nodump` | No dump flag. |
| `noatime` | No atime updates. |
| `sync` | Synchronous writes. |
| `nocow` | No copy-on-write. |
| `nocomp` | No compression. |
| `inline_data` | Use inline data. |
| `inline_dentry` | Use inline directory. |
| `projinherit` | Project quota inheritance. |
| `encrypted` | Encrypted file. |
| `verity` | fs-verity enabled. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Get file flags

```bash
$ f2fs_io getflags /mnt/f2fs/file.txt
flags: 0x00000000
```

### Set file immutable

```bash
$ f2fs_io setflags immutable /mnt/f2fs/file.txt
```

### Enable compression

```bash
$ f2fs_io set_coption lz4 4 /mnt/f2fs/file.txt
```

### Pin file

```bash
$ f2fs_io pinfile on /mnt/f2fs/file.txt
```

### Enable urgent GC

```bash
$ f2fs_io gc_urgent on /mnt/f2fs
```

### Get block allocation info

```bash
$ f2fs_io get_block_info /mnt/f2fs/file.txt
```

## NOTES

- File must be on an F2FS filesystem.
- Some commands require root privileges.
- Compression requires filesystem to be formatted with compression support.
- Pinning affects garbage collection behavior.

## SEE ALSO

**mkfs.f2fs**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
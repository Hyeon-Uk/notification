# btrfsck - Check and repair a Btrfs filesystem

## NAME

**btrfsck** - Check and optionally repair a Btrfs filesystem.

## SYNOPSIS

```bash
btrfsck [OPTIONS] <device>
```

## DESCRIPTION

**btrfsck** (also known as `btrfs check`) is a tool to check and repair a Btrfs filesystem. It can verify the integrity of metadata and data, and attempt to repair errors.

This is a standalone tool that can be used on unmounted filesystems. For online checking, use `btrfs scrub`.

## OPTIONS

### Check Options

| Option | Description |
|--------|-------------|
| `-s <superblock>`, `--super <superblock>` | Use specified superblock copy (0, 1, 2). |
| `-b <bytenr>`, `--backup <bytenr>` | Use backup root at specified bytenr. |
| `-r`, `--tree-root <bytenr>` | Use specified tree root bytenr. |
| `--chunk-root <bytenr>` | Use specified chunk root bytenr. |
| `--log <file>` | Output log to file. |

### Repair Options

| Option | Description |
|--------|-------------|
| `-p`, `--progress` | Show progress. |
| `--repair` | Enable repair mode. |
| `--force` | Force repair. |
| `--init-csum-tree` | Initialize checksum tree. |
| `--init-extent-tree` | Initialize extent tree. |
| `--clear-space-cache <v1|v2>` | Clear space cache. |
| `--check-data-csum` | Check data checksums. |
| `--readonly` | Read-only mode (default). |
| `--mode <mode>` | Check mode: original, lowmem. |

### Output Options

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Verbose output. |
| `-q`, `--quiet` | Suppress output. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | No errors found. |
| `1` | Errors found but not repaired. |
| `2` | Errors found and repaired. |
| `3` | Fatal error during check. |

## EXAMPLES

### Check filesystem (read-only)

```bash
$ btrfsck /dev/sda1
Checking filesystem on /dev/sda1
UUID: 12345678-1234-1234-1234-123456789012
checking extents
checking free space cache
checking fs roots
checking root items
checking dir items
checking inode items
checking ref counts
checking csums
checking dev items
checking dev extents
checking block groups
checking free space tree
Found 0 errors.
```

### Check with progress indicator

```bash
$ btrfsck -p /dev/sda1
Checking filesystem on /dev/sda1
[1/7] checking extents...done
[2/7] checking free space cache...done
[3/7] checking fs roots...done
[4/7] checking root items...done
[5/7] checking dir items...done
[6/7] checking inode items...done
[7/7] checking ref counts...done
Found 0 errors.
```

### Repair filesystem

```bash
$ btrfsck --repair /dev/sda1
enabling repair mode
Checking filesystem on /dev/sda1
...
```

### Check with data checksum verification

```bash
$ btrfsck --check-data-csum /dev/sda1
```

### Use specific superblock copy

```bash
$ btrfsck -s 1 /dev/sda1
```

### Check with low memory mode

```bash
$ btrfsck --mode lowmem /dev/sda1
```

## NOTES

- The filesystem should be unmounted before running btrfsck.
- Repair mode can potentially cause data loss; backup important data first.
- For online checking, use `btrfs scrub` instead.
- `btrfs check` is the primary command name; `btrfsck` is a legacy alias.
- Low memory mode is useful for large filesystems on systems with limited RAM.

## SEE ALSO

**btrfs**(8), **mkfs.btrfs**(8), **fsck.btrfs**(8), **btrfs-scrub**(8)

## PACKAGE

`btrfs-progs`
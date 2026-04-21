# fsck.btrfs - Btrfs filesystem check wrapper

## NAME

**fsck.btrfs** - Wrapper for Btrfs filesystem check.

## SYNOPSIS

```bash
fsck.btrfs [OPTIONS] <device>
```

## DESCRIPTION

**fsck.btrfs** is a wrapper script that is typically called by the system's filesystem check routine at boot time. It exists for compatibility with other filesystem check tools.

For actual filesystem checking, it either returns success immediately or calls `btrfs check` (btrfsck) if requested.

By default, Btrfs does not require filesystem checks at boot time because it maintains its own consistency through copy-on-write (COW) and transaction mechanisms.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-a` | Check all filesystems (ignored for btrfs). |
| `-A` | Check all filesystems in /etc/fstab (ignored for btrfs). |
| `-r` | Report uncorrectable errors (ignored for btrfs). |
| `-s <num>` | Skip check for N superblocks (ignored for btrfs). |
| `-t <fstype>` | Check only specified filesystem type. |
| `-V` | Verbose output. |
| `-y` | Answer yes to all questions (passed to btrfs check). |
| `-n` | Answer no to all questions (read-only check). |
| `--` | End of options. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success (no errors or no check needed). |
| `1` | Errors found or check failed. |
| `2` | Usage error. |

## EXAMPLES

### Check btrfs filesystem

```bash
$ fsck.btrfs /dev/sda1
Btrfs check is skipped on mounted filesystem.
```

### Force check (will run btrfs check on unmounted device)

```bash
$ fsck.btrfs -y /dev/sda1
```

### Verbose output

```bash
$ fsck.btrfs -V /dev/sda1
fsck.btrfs: checking /dev/sda1
```

## NOTES

- Btrfs is designed to not require periodic filesystem checks.
- This wrapper exists primarily for compatibility with system init scripts.
- For manual filesystem checking, use `btrfs check` or `btrfsck` directly.
- Running fsck on a mounted Btrfs filesystem is generally skipped.
- The `-a` option is typically used at boot time and is safe to ignore for Btrfs.

## SEE ALSO

**btrfs**(8), **btrfsck**(8), **mkfs.btrfs**(8), **fsck**(8)

## PACKAGE

`btrfs-progs`
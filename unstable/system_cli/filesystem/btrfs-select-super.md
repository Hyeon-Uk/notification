# btrfs-select-super - Select alternate superblock

## NAME

**btrfs-select-super** - Select an alternate superblock as the primary superblock.

## SYNOPSIS

```bash
btrfs-select-super -s <superblock> <device>
```

## DESCRIPTION

**btrfs-select-super** overwrites the primary superblock with a copy from the specified superblock mirror. Btrfs maintains multiple copies of the superblock, and this tool allows selecting an alternate copy when the primary superblock is corrupted.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-s <superblock>`, `--super <superblock>` | Superblock copy to use (0, 1, or 2). |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Use superblock copy 1 as primary

```bash
$ btrfs-select-super -s 1 /dev/sda1
Using superblock copy 1
```

### Use superblock copy 2 as primary

```bash
$ btrfs-select-super -s 2 /dev/sda1
Using superblock copy 2
```

## NOTES

- The filesystem should be unmounted before using this tool.
- Btrfs maintains 3 copies of the superblock (0, 1, 2).
- Use this tool when the primary superblock (copy 0) is corrupted.
- This is a dangerous operation; use with caution.
- Backup important data before attempting recovery.
- Use `btrfs check` or `btrfs rescue` first to identify the best superblock copy.

## SEE ALSO

**btrfs**(8), **btrfsck**(8), **btrfs-rescue**(8)

## PACKAGE

`btrfs-progs`
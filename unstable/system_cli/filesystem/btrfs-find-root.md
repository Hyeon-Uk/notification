# btrfs-find-root - Find root block of a Btrfs filesystem

## NAME

**btrfs-find-root** - Find the tree root block by searching the filesystem.

## SYNOPSIS

```bash
btrfs-find-root [OPTIONS] <device>
```

## DESCRIPTION

**btrfs-find-root** is a debugging tool that scans a Btrfs filesystem device and identifies potential tree root blocks. It is useful for disaster recovery when the filesystem metadata is corrupted.

This tool searches for tree blocks that could be the root of the filesystem tree and displays their locations and generations.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-g <generation>`, `--generation <generation>` | Find root with specified generation. |
| `-l <level>`, `--level <level>` | Find root with specified level. |
| `-s`, `--skip-ra` | Skip read-ahead. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Find all potential root blocks

```bash
$ btrfs-find-root /dev/sda1
Found tree root at 29696000, generation 100
Found tree root at 29696000, generation 99
Found tree root at 29696000, generation 98
```

### Find root with specific generation

```bash
$ btrfs-find-root -g 100 /dev/sda1
Found tree root at 29696000, generation 100
```

### Find root with specific level

```bash
$ btrfs-find-root -l 0 /dev/sda1
```

## NOTES

- This is a debugging and recovery tool.
- The filesystem should be unmounted before using this tool.
- Use this tool when `btrfs check` cannot find the root due to corruption.
- The output can be used with `btrfs rescue` or `btrfs check` for recovery.
- Multiple root blocks may be found with different generations.

## SEE ALSO

**btrfs**(8), **btrfsck**(8), **btrfs-rescue**(8)

## PACKAGE

`btrfs-progs`
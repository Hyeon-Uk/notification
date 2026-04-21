# btrfs-convert - Convert ext2/3/4 or reiserfs to Btrfs

## NAME

**btrfs-convert** - Convert an ext2/3/4 or reiserfs filesystem to Btrfs.

## SYNOPSIS

```bash
btrfs-convert [OPTIONS] <device>
```

## DESCRIPTION

**btrfs-convert** converts an existing ext2/3/4 or reiserfs filesystem to Btrfs in-place. The original filesystem is preserved as a snapshot that can be used to rollback if needed.

The conversion process creates a Btrfs filesystem with the original filesystem data accessible, and creates a snapshot of the original filesystem state.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-d`, `--no-datasum` | Disable data checksums. |
| `-i`, `--no-inode-map` | Disable inode mapping. |
| `-n`, `--no-xattr` | Disable extended attributes. |
| `-r`, `--rollback` | Rollback to original filesystem. |
| `-l <label>`, `--label <label>` | Set filesystem label. |
| `-L`, `--large-metadata` | Use large metadata blocks. |
| `-N <nr>`, `--nodesize <nr>` | Set nodesize. |
| `-p`, `--progress` | Show progress. |
| `-f`, `--force` | Force conversion. |
| `-v`, `--verbose` | Verbose output. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Convert ext4 to btrfs

```bash
$ btrfs-convert /dev/sda1
creating btrfs metadata.
copy ext4 fs tree
creating ext2fs image file
conversion complete.
```

### Convert with progress indicator

```bash
$ btrfs-convert -p /dev/sda1
[1/3] creating btrfs metadata...done
[2/3] copy ext4 fs tree...done
[3/3] creating ext2fs image file...done
conversion complete.
```

### Rollback to original filesystem

```bash
$ btrfs-convert -r /dev/sda1
rollback to ext4
rollback complete.
```

### Convert with custom label

```bash
$ btrfs-convert -l mydata /dev/sda1
```

### Convert with large metadata

```bash
$ btrfs-convert -L /dev/sda1
```

## NOTES

- The device must be unmounted before conversion.
- The original filesystem is saved as a snapshot named `ext2_saved` (or `reiserfs_saved`).
- After conversion, you can mount the filesystem and verify the data.
- Use `-r` to rollback if needed, but this only works if the snapshot is intact.
- After confirming the conversion is successful, delete the snapshot to reclaim space.
- The conversion requires enough free space on the original filesystem for metadata.

## ROLLBACK

After successful conversion, a snapshot of the original filesystem is preserved. To rollback:

1. Unmount the Btrfs filesystem.
2. Run `btrfs-convert -r /dev/sdXX`.
3. The original filesystem will be restored.

## SEE ALSO

**btrfs**(8), **mkfs.btrfs**(8)

## PACKAGE

`btrfs-progs`
# btrfs-map-logical - Map Btrfs logical addresses to physical

## NAME

**btrfs-map-logical** - Map logical block addresses to physical block addresses.

## SYNOPSIS

```bash
btrfs-map-logical [OPTIONS] <device>
```

## DESCRIPTION

**btrfs-map-logical** maps logical block addresses on a Btrfs filesystem to their corresponding physical block addresses on the underlying device. This is useful for debugging and recovery purposes.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-l <logical>`, `--logical <logical>` | Logical address to map. |
| `-c <copy>`, `--copy <copy>` | Copy number (0, 1, ...). |
| `-o <offset>`, `--offset <offset>` | Offset to add to logical address. |
| `-s <size>`, `--size <size>` | Size of mapping. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Map a logical address

```bash
$ btrfs-map-logical -l 29696000 /dev/sda1
logical 29696000, physical 29696000, length 16384, devid 1
```

### Map with specific copy number

```bash
$ btrfs-map-logical -l 29696000 -c 1 /dev/sda1
logical 29696000, physical 51200000, length 16384, devid 2
```

## NOTES

- This is a debugging tool.
- The filesystem should be unmounted for accurate results.
- Logical addresses are filesystem-internal block addresses.
- Physical addresses are offsets on the underlying block device.
- For RAID profiles, multiple copies may exist on different devices.

## SEE ALSO

**btrfs**(8), **btrfsck**(8)

## PACKAGE

`btrfs-progs`
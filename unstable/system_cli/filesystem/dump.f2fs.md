# dump.f2fs - Dump F2FS filesystem information

## NAME

**dump.f2fs** - Dump filesystem information from an F2FS filesystem.

## SYNOPSIS

```bash
dump.f2fs [OPTIONS] <device>
```

## DESCRIPTION

**dump.f2fs** displays detailed information about an F2FS filesystem structure, including superblock, checkpoint, and segment information. This is useful for debugging and analyzing filesystem status.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-b`, `--superblock` | Dump superblock info. |
| `-c`, `--checkpoint` | Dump checkpoint info. |
| `-n`, `--node` | Dump node info. |
| `-s`, `--sit` | Dump SIT (Segment Information Table) info. |
| `-a`, `--ssa` | Dump SSA (Segment Summary Area) info. |
| `-i <inode>`, `--inode <inode>` | Dump inode info for specified inode number. |
| `-d <nid>`, `--nid <nid>` | Dump node block for specified node id. |
| `-S`, `--scan` | Scan all segments. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Dump superblock

```bash
$ dump.f2fs -b /dev/sda1
Info: sector size = 512
Info: total sectors = 209715200 (102400 MB)
Info: MKFS version = "1.15.0"
Info: Feature = 0 :
Info: checksum_offset = 4092
Info: checkpoint_ver = 1651240415
Info: block_count = 26214399
Info: log_blocksize = 12
Info: log_sectors_per_block = 3
Info: log_blocks_per_seg = 10
Info: segs_per_sec = 1
Info: secs_per_zone = 1
Info: checksum_type = 1
Info: total_sections = 1279
Info: total_node_count = 2457600
Info: valid_node_count = 10
Info: valid_inode_count = 7
Info: main_blkaddr = 512
Info: node_ino = 1
Info: meta_ino = 2
Info: root_ino = 3
Info: next_free_nid = 8
```

### Dump checkpoint info

```bash
$ dump.f2fs -c /dev/sda1
```

### Dump SIT info

```bash
$ dump.f2fs -s /dev/sda1
```

### Dump specific inode

```bash
$ dump.f2fs -i 3 /dev/sda1
```

### Dump node block

```bash
$ dump.f2fs -d 3 /dev/sda1
```

### Scan all segments

```bash
$ dump.f2fs -S /dev/sda1
```

## NOTES

- This is a debugging and analysis tool.
- The filesystem can be mounted or unmounted.
- Dumping all segments with -S may take time on large filesystems.
- Useful for diagnosing filesystem issues.

## SEE ALSO

**mkfs.f2fs**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
# resize.f2fs - Resize F2FS filesystem

## NAME

**resize.f2fs** - Resize an F2FS filesystem.

## SYNOPSIS

```bash
resize.f2fs [OPTIONS] <device> [size]
```

## DESCRIPTION

**resize.f2fs** resizes an existing F2FS filesystem. It can expand or shrink the filesystem, subject to device constraints.

The filesystem should be unmounted before resizing for safety.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-d <level>`, `--debug <level>` | Debug level (0-3). |
| `-f`, `--force` | Force resize. |
| `-i`, `--info` | Show filesystem info. |
| `-s`, `--safe` | Safe resize (verify before resize). |
| `-t <sectors>`, `--target <sectors>` | Target size in sectors. |
| `-V`, `--version` | Show version. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Resize to specific size

```bash
$ resize.f2fs /dev/sda1 50G
```

### Expand to fill device

```bash
$ resize.f2fs /dev/sda1
Info: Segments per section = 1
Info: Sections per zone = 1
Info: sector size = 512
Info: total sectors = 419430400 (204800 MB)
Info: MKFS version = "1.15.0"
Info: FSCK version = "1.15.0"
Info: new size: 209715200 (102400 MB)
Done: 0.004131 seconds
```

### Show filesystem info

```bash
$ resize.f2fs -i /dev/sda1
```

### Safe resize

```bash
$ resize.f2fs -s /dev/sda1 30G
```

### Resize to target sectors

```bash
$ resize.f2fs -t 104857600 /dev/sda1
```

## NOTES

- The filesystem should be unmounted before resizing.
- Shrinking may take time as data needs to be moved.
- Expansion is generally faster than shrinking.
- Ensure the target size is within device limits.
- Backup important data before resizing.

## SEE ALSO

**mkfs.f2fs**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
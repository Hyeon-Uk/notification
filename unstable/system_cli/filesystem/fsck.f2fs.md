# fsck.f2fs - Check and repair F2FS filesystem

## NAME

**fsck.f2fs** - Check and repair an F2FS filesystem.

## SYNOPSIS

```bash
fsck.f2fs [OPTIONS] <device>
```

## DESCRIPTION

**fsck.f2fs** checks and optionally repairs an F2FS filesystem. It verifies the consistency of filesystem metadata and can fix errors.

This tool should be run on an unmounted filesystem for safety, especially when performing repairs.

## OPTIONS

### Check Options

| Option | Description |
|--------|-------------|
| `-a`, `--auto` | Automatic repair (no questions). |
| `-f`, `--force` | Force check even if clean. |
| `-p`, `--preen` | Same as -a. |
| `-r`, `--repair` | Interactive repair mode. |
| `-y`, `--yes` | Answer yes to all questions. |
| `-n`, `--no` | Answer no to all questions (read-only). |

### Output Options

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Verbose output. |
| `-d <level>`, `--debug <level>` | Debug level (0-3). |
| `-V`, `--version` | Show version. |
| `-h`, `--help` | Show help. |

### Special Options

| Option | Description |
|--------|-------------|
| `-c <count>`, `--checkpoint <count>` | Set checkpoint count. |
| `-S`, `--sparse` | Use sparse check. |
| `-C`, `--no_kernel_check` | Skip kernel version check. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | No errors found. |
| `1` | Errors found and fixed. |
| `2` | Errors found, uncorrected. |
| `3` | Filesystem unmounted cleanly (with -n). |
| `4` | Filesystem needs repair. |
| `8` | Operational error. |
| `16` | Usage or syntax error. |
| `32` | Cancelled by user. |
| `128` | Shared library error. |

## EXAMPLES

### Check filesystem (read-only)

```bash
$ fsck.f2fs /dev/sda1
Info: Segments per section = 1
Info: Sections per zone = 1
Info: sector size = 512
Info: total sectors = 209715200 (102400 MB)
Info: MKFS version = "1.15.0"
Info: FSCK version = "1.15.0"
Info: superblock features = 0 : 
Info: superblock encrypt level = 0, salt = 00000000000000000000000000000000
Info: total FS sectors = 209715200 (102400 MB)
Info: Valid checkpoint mirror = 0
Info: Checkpoint clean = 1
Info: Checked dentry block count = 0
Info: Checked node block count = 0
Info: Checked data block count = 0
Done: 0.004131 seconds
```

### Force check even if clean

```bash
$ fsck.f2fs -f /dev/sda1
```

### Automatic repair

```bash
$ fsck.f2fs -a /dev/sda1
```

### Interactive repair

```bash
$ fsck.f2fs -r /dev/sda1
```

### Verbose check

```bash
$ fsck.f2fs -v /dev/sda1
```

### Answer yes to all questions

```bash
$ fsck.f2fs -y /dev/sda1
```

## NOTES

- Run on an unmounted filesystem for safety.
- Automatic mode (-a) will attempt to fix errors without prompting.
- Use -n for a safe, read-only check.
- F2FS maintains checkpoints; fsck validates these.
- The filesystem is marked clean after a successful check.
- Running fsck on a mounted filesystem may cause corruption.

## SEE ALSO

**mkfs.f2fs**(8), **dump.f2fs**(8), **resize.f2fs**(8)

## PACKAGE

`f2fs-tools`
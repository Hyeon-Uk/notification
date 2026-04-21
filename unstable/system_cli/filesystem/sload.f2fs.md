# sload.f2fs - Load files into F2FS filesystem image

## NAME

**sload.f2fs** - Load files from a directory into an F2FS filesystem image.

## SYNOPSIS

```bash
sload.f2fs [OPTIONS] <device> <source_dir>
```

## DESCRIPTION

**sload.f2fs** loads files from a source directory into an existing F2FS filesystem image. This is useful for populating a filesystem image without mounting it.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-f`, `--force` | Force overwrite existing files. |
| `-p`, `--progress` | Show progress. |
| `-t <threads>`, `--threads <threads>` | Number of threads for loading. |
| `-d <level>`, `--debug <level>` | Debug level. |
| `-c`, `--compress` | Compress files during load. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Load directory into filesystem

```bash
$ sload.f2fs /dev/sda1 /source/files
```

### Load with progress

```bash
$ sload.f2fs -p /dev/sda1 /source/files
```

### Load with compression

```bash
$ sload.f2fs -c /dev/sda1 /source/files
```

## NOTES

- Device must contain a valid F2FS filesystem.
- Filesystem should be unmounted.
- Useful for embedded system image creation.
- Supports preserving file attributes.

## SEE ALSO

**mkfs.f2fs**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
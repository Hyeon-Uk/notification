# zipsplit - Split ZIP archives into smaller pieces

## NAME

**zipsplit** - Split a ZIP archive into smaller ZIP archives.

## SYNOPSIS

```bash
zipsplit [-tips] [-n size] [-b path] zipfile
```

## DESCRIPTION

**zipsplit** splits a ZIP archive into smaller ZIP archives, each containing one or more files. This is useful for distributing large archives on media with size limitations (e.g., floppy disks, email attachments).

## OPTIONS

| Option | Description |
|--------|-------------|
| `-n size` | Make each output archive at most `size` bytes. |
| `-t` | Report how many files will be output. No output created. |
| `-i` | Make index (first output) a separate file. |
| `-p` | Pause between each output file. |
| `-s` | Run silently (suppress most output). |
| `-b path` | Use path for temporary file. |
| `-h` | Show help. |
| `-v` | Show version. |
| `-L` | Show license. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The operation completed successfully. |
| `1` | Error. File not found or I/O error. |
| `2` | Error. Memory allocation failure. |
| `3` | Error. Not a valid ZIP archive. |
| `4` | Error. File too large to split. |

## EXAMPLES

### Split archive into default-sized pieces

```bash
$ zipsplit large_archive.zip
```

### Split with size limit (1.44MB for floppy)

```bash
$ zipsplit -n 1457664 large_archive.zip
```

### Test how many files will be created

```bash
$ zipsplit -t archive.zip
```

### Split with pause between files

```bash
$ zipsplit -p archive.zip
```

### Split with index file

```bash
$ zipsplit -i archive.zip
```

### Use specific temp directory

```bash
$ zipsplit -b /tmp archive.zip
```

## NOTES

- Creates output files named `prefix01.zip`, `prefix02.zip`, etc.
- Each output archive is a valid ZIP archive.
- Useful for transferring large archives on size-limited media.
- The `-t` option only reports, does not create files.

## SEE ALSO

**zip**(1), **unzip**(1), **zipcloak**(1), **zipnote**(1)

## PACKAGE

`zip`
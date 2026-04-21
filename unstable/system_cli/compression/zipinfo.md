# zipinfo - List detailed information about ZIP archives

## NAME

**zipinfo** - List detailed information about files in a ZIP archive.

## SYNOPSIS

```bash
zipinfo [-12smlvhtzMCT] zipfile [file(s) ...] [-x xlist]
```

## DESCRIPTION

**zipinfo** lists technical information about files in a ZIP archive, including compression information, file sizes, timestamps, and permissions. It provides more detailed output than `unzip -l`.

## OPTIONS

### Format Options

| Option | Description |
|--------|-------------|
| `-1` | Filenames only, one per line. |
| `-2` | Like `-1`, but allows `-h/-t/-z` options. |
| `-s` | Short format (default). |
| `-m` | Medium Unix "ls -l" format. |
| `-l` | Long Unix "ls -l" format. |
| `-v` | Verbose, multi-page format. |

### Header and Footer Options

| Option | Description |
|--------|-------------|
| `-h` | Print header line. |
| `-t` | Print totals for listed files. |
| `-z` | Print zipfile comment. |
| `-T` | Print file times in sortable decimal format. |

### Behavior Options

| Option | Description |
|--------|-------------|
| `-C` | Be case-insensitive. |
| `-M` | Page output through built-in "more". |
| `-x xlist` | Exclude filenames in xlist from listing. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Normal completion. |
| `1` | Error. Warning or non-fatal error. |
| `2` | Error. Generic error. |
| `3` | Error. Severe error. |

## EXAMPLES

### List files in short format

```bash
$ zipinfo archive.zip
```

### List filenames only

```bash
$ zipinfo -1 archive.zip
```

### Long listing format

```bash
$ zipinfo -l archive.zip
```

### Verbose multi-page format

```bash
$ zipinfo -v archive.zip
```

### Show header and totals

```bash
$ zipinfo -h -t archive.zip
```

### List specific files

```bash
$ zipinfo archive.zip "*.txt"
```

### Exclude files from listing

```bash
$ zipinfo archive.zip -x "*.log"
```

### Sortable timestamps

```bash
$ zipinfo -T archive.zip
```

## NOTES

- Provides more detailed technical info than `unzip -l`.
- Shows compression method, ratio, and internal file attributes.
- Default format includes: permissions, file size, compression method, date, time, and filename.
- Same program as `unzip -Z`.

## SEE ALSO

**unzip**(1), **zip**(1), **zipgrep**(1)

## PACKAGE

`unzip`
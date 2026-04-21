# zip - Package and compress files into ZIP archives

## NAME

**zip** - Package and compress files into a ZIP archive.

## SYNOPSIS

```bash
zip [-options] [-b path] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]
```

## DESCRIPTION

**zip** is a compression and file packaging utility. It puts one or more compressed files into a single ZIP archive, along with information about the files. An entire directory structure can be packed into a ZIP archive with a single command.

## OPTIONS

### Compression Options

| Option | Description |
|--------|-------------|
| `-0` | Store only (no compression). |
| `-1` | Compress faster. |
| `-9` | Compress better. |
| `-Z` | Set compression method (store, deflate, bzip2). |

### Archive Options

| Option | Description |
|--------|-------------|
| `-r` | Recurse into directories. |
| `-e` | Encrypt (prompt for password). |
| `-P password` | Encrypt with password. |
| `-m` | Move files into archive (delete originals). |
| `-j` | Junk directory paths. |
| `-D` | Do not add directory entries. |
| `-z` | Add archive comment. |

### File Selection Options

| Option | Description |
|--------|-------------|
| `-x files` | Exclude files. |
| `-i files` | Include only files. |
| `-f` | Freshen (only files that exist in archive). |
| `-u` | Update (add or update files). |
| `-d` | Delete entries from archive. |

### Other Options

| Option | Description |
|--------|-------------|
| `-q` | Quiet mode. |
| `-v` | Verbose mode or display version. |
| `-t date` | Include only files modified after date. |
| `-b path` | Use path for temporary archive. |
| `-@` | Read file list from stdin. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Normal; no errors. |
| `1` | Warning errors; not fatal. |
| `2` | Generic error. |
| `3` | Severe error. |
| `4-18` | Specific error conditions. |

## EXAMPLES

### Create archive from directory

```bash
$ zip -r archive.zip /path/to/directory
```

### Create encrypted archive

```bash
$ zip -e -r archive.zip /path/to/directory
```

### Create archive with password

```bash
$ zip -P secret archive.zip file1.txt file2.txt
```

### Add files to existing archive

```bash
$ zip archive.zip newfile.txt
```

### Create archive excluding files

```bash
$ zip -r archive.zip /path/to/directory -x "*.log"
```

### Archive specific file types

```bash
$ zip archive.zip *.txt
```

### Update archive with new files

```bash
$ zip -u archive.zip modified_file.txt
```

### Delete file from archive

```bash
$ zip -d archive.zip unwanted.txt
```

### Read file list from stdin

```bash
$ find . -name "*.c" | zip -@ source.zip
```

## NOTES

- Compression level 1 is fastest; level 9 provides best compression.
- Default compression level is typically 6.
- Supports password encryption (standard or strong encryption).
- Can create self-extracting archives when combined with unzipsfx.

## SEE ALSO

**unzip**(1), **funzip**(1), **zipcloak**(1), **zipnote**(1)

## PACKAGE

`zip`
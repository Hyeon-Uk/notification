# unzip - Extract files from ZIP archives

## NAME

**unzip** - List, test, and extract compressed files from a ZIP archive.

## SYNOPSIS

```bash
unzip [-Z] [-cflptuvz[abjnoqsCDKLMUVWX$]] file[.zip] [file(s) ...] [-x file(s) ...] [-d exdir]
```

## DESCRIPTION

**unzip** lists, tests, or extracts files from a ZIP archive. The default behavior is to extract files from the specified ZIP archive to the current directory.

## OPTIONS

### Action Options

| Option | Description |
|--------|-------------|
| `-p` | Extract files to pipe (stdout). No messages. |
| `-l` | List archive files (short format). |
| `-t` | Test compressed archive data integrity. |
| `-u` | Update files, create if necessary. |
| `-f` | Freshen existing files only, create none. |
| `-v` | List verbosely or show version info. |
| `-z` | Display archive comment only. |
| `-T` | Timestamp archive to latest file. |

### Behavior Options

| Option | Description |
|--------|-------------|
| `-n` | Never overwrite existing files. |
| `-o` | Overwrite files without prompting. |
| `-q` | Quiet mode (`-qq` for quieter). |
| `-a` | Auto-convert any text files. |
| `-aa` | Treat ALL files as text. |
| `-j` | Junk paths (strip all directories). |
| `-j=N` | Strip top-N directory levels. |

### Name Handling Options

| Option | Description |
|--------|-------------|
| `-C` | Match filenames case-insensitively. |
| `-L` | Make (some) names lowercase. |
| `-U` | Use escapes for all non-ASCII Unicode. |
| `-UU` | Ignore any Unicode fields. |

### Permission Options

| Option | Description |
|--------|-------------|
| `-X`, `-k` | Restore UID/GID and permissions. |
| `-K` | Keep setuid/setgid/tacky permissions. |
| `-D-` | Restore directory times (`-D` for no). |
| `-V` | Retain VMS version numbers. |

### Other Options

| Option | Description |
|--------|-------------|
| `-M` | Pipe through `more` pager. |
| `-Z` | ZipInfo mode (see zipinfo). |
| `-d exdir` | Extract files into directory `exdir`. |
| `-x file(s)` | Exclude specified files. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Normal; no errors. |
| `1` | Warning errors; not fatal. |
| `2` | Generic error. |
| `3` | Severe error. |
| `4-8` | Memory or file system errors. |
| `9` | Entry not found. |
| `10` | Disk full. |
| `11` | No more files. |

## EXAMPLES

### Extract all files from archive

```bash
$ unzip archive.zip
```

### List files in archive

```bash
$ unzip -l archive.zip
```

### Extract to specific directory

```bash
$ unzip archive.zip -d /path/to/directory
```

### Extract specific files

```bash
$ unzip archive.zip file1.txt file2.txt
```

### Extract excluding files

```bash
$ unzip archive.zip -x "*.log"
```

### Test archive integrity

```bash
$ unzip -t archive.zip
```

### Overwrite without prompting

```bash
$ unzip -o archive.zip
```

### Quiet extraction

```bash
$ unzip -q archive.zip
```

## NOTES

- Matches file specifications against archive entries in order.
- Supports wildcards for file specifications.
- Can handle password-protected archives (prompts for password).

## SEE ALSO

**zip**(1), **funzip**(1), **zipinfo**(1), **gzip**(1)

## PACKAGE

`unzip`
# tar - Archive files

## NAME

**tar** - Archive and extract files (tape archive).

## SYNOPSIS

```bash
tar [OPTIONS] [FILE...]
```

## DESCRIPTION

**tar** creates, maintains, modifies, and extracts files from archive files known as tarfiles. The name comes from "tape archive" but tarfiles can be stored on any medium.

## OPTIONS

### Operation Options

| Option | Description |
|--------|-------------|
| `-A`, `--catenate`, `--concatenate` | Append tar files to an archive. |
| `-c`, `--create` | Create a new archive. |
| `-d`, `--diff`, `--compare` | Find differences between archive and file system. |
| `--delete` | Delete from archive. |
| `-r`, `--append` | Append files to archive. |
| `-t`, `--list` | List contents of archive. |
| `-u`, `--update` | Append files newer than archive copy. |
| `-x`, `--extract`, `--get` | Extract from archive. |

### Compression Options

| Option | Description |
|--------|-------------|
| `-a`, `--auto-compress` | Use archive suffix to determine compression. |
| `-I`, `--use-compress-program <cmd>` | Filter through specified command. |
| `-j`, `--bzip2` | Filter through bzip2. |
| `-J`, `--xz` | Filter through xz. |
| `-z`, `--gzip`, `--gunzip`, `--ungzip` | Filter through gzip. |
| `--lzip` | Filter through lzip. |
| `--lzma` | Filter through lzma. |
| `--lzop` | Filter through lzop. |
| `--no-auto-compress` | Do not use archive suffix. |
| `-Z`, `--compress`, `--uncompress` | Filter through compress. |
| `--zstd` | Filter through zstd. |

### File Selection Options

| Option | Description |
|--------|-------------|
| `-C <dir>`, `--directory <dir>` | Change to directory before operation. |
| `-f <file>`, `--file <file>` | Use specified archive file. |
| `--exclude <pattern>` | Exclude files matching pattern. |
| `--exclude-from <file>` | Exclude patterns from file. |
| `-T <file>`, `--files-from <file>` | Read file names from file. |
| `-X <file>`, `--exclude-from <file>` | Exclude patterns from file. |

### Output Options

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Verbose output. |
| `-vv` | Very verbose output. |
| `--index-file <file>` | Send verbose output to file. |

### Archive Options

| Option | Description |
|--------|-------------|
| `-k`, `--keep-old-files` | Don't overwrite existing files. |
| `-m`, `--touch` | Don't extract modification time. |
| `-M`, `--multi-volume` | Create/list/extract multi-volume archive. |
| `-O`, `--to-stdout` | Extract files to stdout. |
| `--overwrite` | Overwrite existing files. |
| `--overwrite-dir` | Overwrite metadata of existing directories. |
| `--remove-files` | Remove files after adding to archive. |
| `-W`, `--verify` | Verify archive after writing. |
| `-w`, `--interactive`, `--confirmation` | Ask for confirmation. |

### Permission Options

| Option | Description |
|--------|-------------|
| `--no-same-owner` | Extract files as yourself. |
| `--no-same-permissions` | Apply user's umask. |
| `--same-owner` | Try to preserve ownership. |
| `--same-permissions` | Preserve permissions. |
| `-p`, `--preserve-permissions` | Same as --same-permissions. |
| `--preserve` | Same as -p and -s. |
| `-s`, `--same-order`, `--preserve-order` | Sort names to extract. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Successful completion. |
| `1` | Some files differ. |
| `2` | Fatal error. |

## EXAMPLES

### Create archive from directory

```bash
$ tar -cvf archive.tar /path/to/directory
```

### Create gzipped archive

```bash
$ tar -czvf archive.tar.gz /path/to/directory
```

### Create bzipped archive

```bash
$ tar -cjvf archive.tar.bz2 /path/to/directory
```

### Create xz compressed archive

```bash
$ tar -cJvf archive.tar.xz /path/to/directory
```

### List archive contents

```bash
$ tar -tvf archive.tar.gz
drwxr-xr-x user/group 0 2024-01-01 10:00 directory/
-rw-r--r-- user/group 1234 2024-01-01 10:00 directory/file.txt
```

### Extract archive

```bash
$ tar -xvf archive.tar.gz
```

### Extract to specific directory

```bash
$ tar -xvf archive.tar.gz -C /path/to/extract/
```

### Extract specific files

```bash
$ tar -xvf archive.tar.gz file1.txt file2.txt
```

### Create archive excluding files

```bash
$ tar -czvf archive.tar.gz --exclude "*.log" /path/to/directory
```

### Append files to existing archive

```bash
$ tar -rvf archive.tar newfile.txt
```

### Verify archive integrity

```bash
$ tar -tvWf archive.tar
```

### Create archive from file list

```bash
$ tar -czvf archive.tar.gz -T filelist.txt
```

## NOTES

- Always use `-f` to specify the archive file.
- Use `-v` to see progress during operations.
- Compression suffix determines compression when using `-a`.
- For large backups, consider using `-M` for multi-volume archives.
- Use `--exclude` patterns carefully to avoid unintended exclusions.

## SEE ALSO

**gzip**(1), **bzip2**(1), **xz**(1), **zip**(1)

## PACKAGE

`tar`
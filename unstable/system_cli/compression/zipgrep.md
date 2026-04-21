# zipgrep - Search inside ZIP archives

## NAME

**zipgrep** - Search for a pattern within files in a ZIP archive.

## SYNOPSIS

```bash
zipgrep [grep_options] pattern zipfile [file(s) ...]
```

## DESCRIPTION

**zipgrep** searches for a pattern within one or more files in a ZIP archive. It combines the functionality of `unzip -p` and `grep`, allowing you to search the contents of compressed files without extracting them.

## OPTIONS

Accepts standard `grep` options:

| Option | Description |
|--------|-------------|
| `-i` | Case-insensitive search. |
| `-v` | Invert match (show non-matching lines). |
| `-n` | Show line numbers. |
| `-l` | List only filenames with matches. |
| `-c` | Count matching lines per file. |
| `-e pattern` | Specify pattern explicitly. |
| `-E` | Extended regular expression. |
| `-F` | Fixed string (no regex). |

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `pattern` | Regular expression pattern to search for. |
| `zipfile` | ZIP archive to search. |
| `file(s)` | Optional: specific files within archive to search. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Matches found. |
| `1` | No matches found. |
| `2` | Error. File not found or invalid arguments. |

## EXAMPLES

### Search for pattern in all files

```bash
$ zipgrep "TODO" project.zip
```

### Case-insensitive search

```bash
$ zipgrep -i "error" archive.zip
```

### Search in specific file types

```bash
$ zipgrep "function" archive.zip "*.c"
```

### Show line numbers

```bash
$ zipgrep -n "TODO" archive.zip
```

### List matching files only

```bash
$ zipgrep -l "pattern" archive.zip
```

### Count matches per file

```bash
$ zipgrep -c "error" archive.zip
```

## NOTES

- Uses `egrep` internally for pattern matching.
- Automatically decompresses files to stdout for searching.
- Does not modify the archive.
- Useful for searching source code archives.

## SEE ALSO

**grep**(1), **zip**(1), **unzip**(1), **zipinfo**(1)

## PACKAGE

`unzip`
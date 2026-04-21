# zipnote - Modify ZIP archive comments

## NAME

**zipnote** - View, add, or modify comments in a ZIP archive.

## SYNOPSIS

```bash
zipnote [-w] zipfile
```

## DESCRIPTION

**zipnote** reads or writes the comments in a ZIP archive. It can display the global archive comment and individual file comments, or write new comments back to the archive.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-w` | Write mode. Read comments from stdin and write to archive. |
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

## EXAMPLES

### View archive comments

```bash
$ zipnote archive.zip
```

### Add archive comment

```bash
$ echo "@ This is the archive comment" | zipnote -w archive.zip
```

### Write comments from file

```bash
$ zipnote archive.zip > comments.txt
# Edit comments.txt
$ zipnote -w archive.zip < comments.txt
```

### Comment file entry

```bash
$ echo "@ File: readme.txt
@ Comment: This is the README file" | zipnote -w archive.zip
```

## NOTES

- Output format: `@ File: filename` followed by `@ Comment: text`.
- Global archive comment appears as `@ (comment above)`.
- Use `-w` to write modified comments back to archive.
- Comments are read from stdin when writing.

## SEE ALSO

**zip**(1), **unzip**(1), **zipcloak**(1)

## PACKAGE

`zip`
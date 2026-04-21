# funzip - Filter unzip from stdin

## NAME

**funzip** - Extract first member from a zip archive on stdin.

## SYNOPSIS

```bash
funzip [-password] [file[.zip]]
```

## DESCRIPTION

**funzip** acts as a filter to extract the first member from a zip archive. It reads a zip archive from stdin and extracts the first file to stdout. It is primarily useful for extracting a single file from a zip archive in a pipeline.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-password` | Specify password for encrypted zip entries. |
| `file[.zip]` | Optional zip file to read (instead of stdin). |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Extraction completed without errors. |
| `1` | Error. No zip data found, decryption failed, or I/O error. |
| `2` | Error. Unexpected end of zip file. |
| `3` | Error. Memory allocation failure. |

## EXAMPLES

### Extract first file from piped zip

```bash
$ cat archive.zip | funzip > output.txt
```

### Extract with password

```bash
$ cat archive.zip | funzip -secret > output.txt
```

### Download and extract in one command

```bash
$ curl -s http://example.com/file.zip | funzip > output.txt
```

## NOTES

- Only the first member of the zip archive is extracted.
- If no file argument is given, reads from stdin.
- Useful for extracting single-file archives from pipelines.
- Cannot list or extract multiple files - use `unzip` for that.

## SEE ALSO

**unzip**(1), **zip**(1), **gzip**(1)

## PACKAGE

`unzip`
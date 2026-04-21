# zipcloak - Encrypt or decrypt ZIP archives

## NAME

**zipcloak** - Encrypt or decrypt entries in a ZIP archive.

## SYNOPSIS

```bash
zipcloak [-d] [-b path] [-h] [-v] [-L] zipfile
```

## DESCRIPTION

**zipcloak** encrypts or decrypts the contents of a ZIP archive. Without options, it encrypts all unencrypted entries in the archive. With the `-d` option, it decrypts all encrypted entries.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-d` | Decrypt encrypted entries. |
| `-b path` | Use path for temporary file. |
| `-h` | Show help. |
| `-v` | Show version information. |
| `-L` | Show software license. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The operation completed successfully. |
| `1` | Error. File not found, I/O error, or incorrect password. |
| `2` | Error. Memory allocation failure. |
| `3` | Error. Not a valid ZIP archive. |

## EXAMPLES

### Encrypt all entries in archive

```bash
$ zipcloak archive.zip
```

### Decrypt all entries in archive

```bash
$ zipcloak -d archive.zip
```

### Use specific temp directory

```bash
$ zipcloak -b /tmp archive.zip
```

## NOTES

- Prompts for password when encrypting or decrypting.
- Encrypts all unencrypted entries by default.
- Creates a new copy of the archive; original is replaced.
- Supports standard ZIP encryption only.

## SEE ALSO

**zip**(1), **unzip**(1), **zipnote**(1)

## PACKAGE

`zip`
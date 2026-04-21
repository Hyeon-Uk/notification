# unzipsfx - Self-extracting ZIP stub

## NAME

**unzipsfx** - Self-extracting stub for ZIP archives.

## SYNOPSIS

```bash
archive.exe [-cfptuvz[abjnoqsCDKLMUVXYZ$]] [file(s) ...] [-x file(s) ...]
```

## DESCRIPTION

**unzipsfx** is a self-extracting stub program that, when prepended to a ZIP archive, creates a self-extracting executable. The resulting file can unpack itself without requiring an external unzip utility.

To create a self-extracting archive:
```bash
cat /usr/bin/unzipsfx archive.zip > archive.exe
chmod +x archive.exe
```

## OPTIONS

The options are similar to `unzip` but are applied when the self-extracting archive is run:

| Option | Description |
|--------|-------------|
| `-c` | Extract files to stdout. |
| `-f` | Freshen existing files only. |
| `-p` | Extract files to pipe (stdout). |
| `-t` | Test archive integrity. |
| `-u` | Update files, create if necessary. |
| `-v` | List files verbosely. |
| `-z` | Display archive comment. |
| `-n` | Never overwrite existing files. |
| `-o` | Overwrite without prompting. |
| `-q` | Quiet mode. |
| `-a` | Auto-convert text files. |
| `-j` | Junk paths (strip directories). |
| `-C` | Match filenames case-insensitively. |
| `-L` | Make names lowercase. |
| `-d exdir` | Extract to directory `exdir`. |
| `-x file(s)` | Exclude specified files. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Normal; no errors. |
| `1` | Warning errors; not fatal. |
| `2` | Generic error. |
| `3` | Severe error. |

## EXAMPLES

### Create self-extracting archive

```bash
$ cat /usr/bin/unzipsfx myapp.zip > myapp.exe
$ chmod +x myapp.exe
```

### Run self-extracting archive

```bash
$ ./myapp.exe
```

### Extract to specific directory

```bash
$ ./myapp.exe -d /opt/myapp
```

### Extract quietly

```bash
$ ./myapp.exe -q
```

## NOTES

- The stub must be prepended to a valid ZIP archive.
- The resulting executable runs on the same platform as the stub.
- Useful for distributing software to systems without unzip installed.
- See `zip` documentation for creating self-extracting archives.

## SEE ALSO

**unzip**(1), **zip**(1), **zipcloak**(1)

## PACKAGE

`unzip`
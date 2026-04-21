# localedef

**NAME**

localedef - Compile locale definition files

**SYNOPSIS**

```bash
localedef [OPTIONS] NAME
```

**DESCRIPTION**

`localedef` reads a locale definition file and compiles it into a locale archive that can be used by the system. The compiled locale can then be set using environment variables or locale functions.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-c`, `--force` | Continue even if warnings occur |
| `-f CHARMAP`, `--charmap=CHARMAP` | Use character map file |
| `-i FILE`, `--inputfile=FILE` | Locale definition input file |
| `-u FILE`, `--repertoire-map=FILE` | Repertoire map file |
| `-A FILE`, `--alias-file=FILE` | Locale alias file |
| `--no-archive` | Store locale as separate files |
| `--no-warnings` | Suppress warnings |
| `--posix` | Strictly conform to POSIX |
| `-v`, `--verbose` | Verbose output |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Create a locale from standard locale definition:

```bash
localedef -f UTF-8 -i en_US en_US.UTF-8
```

2. Create a locale with force option:

```bash
localedef -c -f UTF-8 -i ko_KR ko_KR.UTF-8
```

3. Create locale without archive:

```bash
localedef --no-archive -f UTF-8 -i de_DE de_DE.UTF-8
```

**FILES**

| File | Description |
|------|-------------|
| `/usr/share/i18n/charmaps/` | Character map files |
| `/usr/share/i18n/locales/` | Locale definition files |
| `/usr/lib/locale/` | Compiled locale archive |
| `/usr/lib/locale/locale-archive` | Main locale archive |

**EXIT STATUS**

| Code | Description |
|------|-------------|
| 0 | Successful compilation |
| 1 | Error in locale definition |
| 2 | Cannot open or read files |
| 3 | Insufficient memory |

**SEE ALSO**

locale(1), charmap(5), locale(5)

**PACKAGE**

glibc
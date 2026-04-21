# iconvconfig

**NAME**

iconvconfig - Create iconv module configuration cache

**SYNOPSIS**

```bash
iconvconfig [OPTIONS] [DIRECTORY...]
```

**DESCRIPTION**

`iconvconfig` creates a fast-loading iconv module configuration cache file. This cache speeds up the loading of iconv modules when applications use iconv for character encoding conversion.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-o FILE`, `--output=FILE` | Output file name (default: iconv.cache) |
| `--prefix=PREFIX` | Add PREFIX to all directory names |
| `-n`, `--dry-run` | Don't write output file |
| `-v`, `--verbose` | Verbose output |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Create default cache:

```bash
iconvconfig
```

2. Create cache to specific file:

```bash
iconvconfig -o /usr/lib/gconv/iconv.cache
```

3. Create cache for specific directories:

```bash
iconvconfig /usr/lib/gconv
```

**FILES**

- `/usr/lib/gconv/` - Iconv module directory
- `/usr/lib/gconv/iconv.cache` - Cache file

**NOTES**

- This command is typically run during system installation or updates.
- The cache file improves iconv initialization speed.
- Root privileges are usually required to update the system cache.

**SEE ALSO**

iconv(1), iconv(3)

**PACKAGE**

glibc
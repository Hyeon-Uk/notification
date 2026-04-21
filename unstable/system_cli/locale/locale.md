# locale

**NAME**

locale - Get locale-specific information

**SYNOPSIS**

```bash
locale [OPTIONS]
locale [OPTIONS] -a
locale [OPTIONS] -m
locale [OPTIONS] NAME...
```

**DESCRIPTION**

`locale` displays information about the current locale settings or available locales. It can show locale categories, character maps, and all locale-related information.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-a`, `--all-locales` | List all available locales |
| `-m`, `--charmaps` | List available character maps |
| `-c`, `--category-name` | Print category names with values |
| `-k`, `--keyword-name` | Print keyword names with values |
| `-v`, `--verbose` | Verbose output |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**LOCALE CATEGORIES**

| Category | Description |
|----------|-------------|
| LC_CTYPE | Character classification and case conversion |
| LC_NUMERIC | Non-monetary numeric formats |
| LC_TIME | Date and time formats |
| LC_COLLATE | String collation order |
| LC_MONETARY | Monetary formats |
| LC_MESSAGES | Language of messages |
| LC_PAPER | Paper size |
| LC_NAME | Name formats |
| LC_ADDRESS | Address formats |
| LC_TELEPHONE | Telephone number formats |
| LC_MEASUREMENT | Measurement units |
| LC_IDENTIFICATION | Locale identification |
| LC_ALL | All categories (overrides individual settings) |

**EXAMPLES**

1. Show current locale settings:

```bash
locale
```

2. List all available locales:

```bash
locale -a
```

3. List available character maps:

```bash
locale -m
```

4. Show specific locale variable:

```bash
locale LC_TIME
```

5. Show all keywords for a category:

```bash
locale -k LC_TIME
```

6. Show category name with values:

```bash
locale -c LC_NUMERIC
```

**ENVIRONMENT**

The following environment variables affect locale settings:
- `LANG` - Default locale
- `LC_ALL` - Override all LC_* variables
- `LC_*` - Individual category settings

**FILES**

- `/usr/lib/locale/` - Locale archive and definitions
- `/usr/share/i18n/` - Locale definition files

**SEE ALSO**

localedef(1), charmap(5), locale(7)

**PACKAGE**

glibc
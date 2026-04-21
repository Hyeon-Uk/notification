# NAME

theme-cmd - Tizen theme management command-line tool

# SYNOPSIS

```bash
theme-cmd [OPTIONS]
```

# DESCRIPTION

`theme-cmd` is the Tizen interactive command-line client for the underlying `tizen-theme-manager` infrastructure. It enables explicit querying and applying of themes globally on the system.

# OPTIONS

| Option | Long Option | Argument | Description |
|--------|-------------|----------|-------------|
| `-s` | `--set-default` | `<theme_id>` | Set the default theme by ID |
| `-g` | `--get` | `<theme_id>` | Get specific theme info by ID |
| `-c` | `--get-current` | - | Get current active theme info |
| `-l` | `--list` | - | Print list of installed themes |
| `-h` | `--help` | - | Display help message |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Prints theme ID lists or metadata dictionaries regarding installed visual themes

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Operation completed successfully |
| nonzero | Error - Execution failed |

# ENVIRONMENT

- Communicates with the Theme Manager daemon via D-Bus

# EXAMPLES

## List all installed themes

```bash
$ theme-cmd -l
```

## Get current active theme

```bash
$ theme-cmd -c
```

## Get specific theme information

```bash
$ theme-cmd -g <theme_id>
```

## Set default theme

```bash
$ theme-cmd -s <theme_id>
```

# SEE ALSO

dbus(1)

# PACKAGE

tizen-theme-manager
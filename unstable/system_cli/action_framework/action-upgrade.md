# NAME

action-upgrade - Action Framework database schema migration and upgrade tool

# SYNOPSIS

```bash
action-upgrade [OPTIONS]
```

# DESCRIPTION

`action-upgrade` handles database schema migration and upgrade for the Tizen Action Framework. It updates the action database structure when the system or framework is upgraded, with support for dry-run previews and verbose logging.

# OPTIONS

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-h` | `--help` | Show help message |
| `-v` | `--verbose` | Enable verbose output during upgrade |
| `-d` | `--dry-run` | Show what would be changed without modifying the database |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Reports migration steps and results. With `--dry-run`, shows planned changes without applying them.

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Database upgrade completed or dry-run previewed successfully |
| 1 | Error - Upgrade failed due to database error or incompatible schema |

# FILES

- `/var/db/action.db` - Action Framework SQLite database location

# EXAMPLES

## Preview upgrade changes without applying

```bash
$ action-upgrade --dry-run --verbose
```

## Run the database upgrade

```bash
$ action-upgrade
```

## Run with verbose output

```bash
$ action-upgrade -v
```

# SEE ALSO

action-db-recovery(1), action-tool(1)

# PACKAGE

action-frame
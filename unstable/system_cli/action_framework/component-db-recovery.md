# NAME

component-db-recovery - Component Framework database integrity check and recovery tool

# SYNOPSIS

```bash
component-db-recovery
```

# DESCRIPTION

`component-db-recovery` performs integrity checking and recovery of the Component Framework database. It verifies the `component_info` and `component_localized_info` tables for both global and user databases.

# OPTIONS

This tool does not accept any command-line options.

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Outputs table-by-table integrity check results

**Example output:**
```
__check_table(177): component_info component_info
__check_table(177): component_localized_info component_localized_info
```

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - All database tables passed integrity check |
| 1 | Error - Database corruption detected |

# FILES

- `/var/db/component.db` - Component Framework SQLite database location

# EXAMPLES

## Run integrity check

```bash
$ component-db-recovery
```

# SEE ALSO

action-db-recovery(1), appsvc-db-recovery(1)

# PACKAGE

component-manager
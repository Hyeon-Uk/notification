# NAME

appsvc-db-recovery - Application Service database integrity check and recovery tool

# SYNOPSIS

```bash
appsvc-db-recovery
```

# DESCRIPTION

`appsvc-db-recovery` performs integrity checking and recovery of the Application Service (appsvc) database. The database stores alias info, allowed info, app service dispatch rules, and boot sequence data. This tool runs automatically during system initialization.

# OPTIONS

This tool does not accept any command-line options.

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Outputs table-by-table integrity check results

**Example output:**
```
__check_table(177): alias_info alias_info
__check_table(177): alias_info_for_uid alias_info_for_uid
__check_table(177): allowed_info allowed_info
__check_table(177): appsvc appsvc
__check_table(177): boot_sequence boot_sequence
```

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - All database tables passed integrity check |
| 1 | Error - Database corruption detected and recovery attempted |

# FILES

- `/var/db/appsvc.db` - Application Service SQLite database

## Database Tables Checked

| Table | Description |
|-------|-------------|
| `alias_info` | Alias information |
| `alias_info_for_uid` | Per-user alias information |
| `allowed_info` | Allowed application information |
| `appsvc` | App service dispatch rules |
| `boot_sequence` | Boot sequence data |

# EXAMPLES

## Run database integrity check

```bash
$ appsvc-db-recovery
```

# SEE ALSO

action-db-recovery(1), component-db-recovery(1)

# PACKAGE

appsvc
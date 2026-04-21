# NAME

action-db-recovery - Action Framework database integrity check and recovery tool

# SYNOPSIS

```bash
action-db-recovery
```

# DESCRIPTION

`action-db-recovery` performs integrity checking and recovery of the Action Framework database. It verifies database consistency and repairs corrupted entries. This tool runs automatically during system initialization or can be invoked manually for maintenance purposes.

# OPTIONS

This tool does not accept any command-line options.

# OUTPUT FORMAT

- **Format:** Plain Text
- **Structure Details:** Outputs the result of the integrity check

**Example output:**
```
Integrity check for action db is ok
```

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Database integrity check passed or recovery completed successfully |
| 1 | Error - Database corruption could not be repaired |

# FILES

- `/var/db/action.db` - Action Framework SQLite database location

# EXAMPLES

## Run integrity check

```bash
$ action-db-recovery
```

**Expected output:**
```
Integrity check for action db is ok
```

## Run during system maintenance

```bash
$ sudo action-db-recovery
```

# SEE ALSO

action-tool(1), alarmmgr_tool(1)

# PACKAGE

action-frame
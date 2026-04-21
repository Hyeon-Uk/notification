# widget_recovery

## NAME

widget_recovery - Widget database recovery tool

## SYNOPSIS

```
widget_recovery
```

## DESCRIPTION

`widget_recovery` is a utility tool for recovering and repairing the widget database on Tizen systems. It scans for installed widgets and rebuilds the widget database if it becomes corrupted or inconsistent.

This tool is typically run automatically during system boot or can be invoked manually when widget-related issues are detected.

## FUNCTIONALITY

The widget recovery tool performs the following operations:

1. Scans widget installation directories
2. Validates widget manifests
3. Rebuilds widget database entries
4. Fixes database inconsistencies
5. Removes orphaned widget entries

## USAGE

Run the tool without arguments:

```bash
widget_recovery
```

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success (recovery completed or no issues found) |
| Non-zero | Error occurred during recovery |

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/apps/` | Widget installation directory |
| `/opt/usr/dbspace/.widget.db` | Widget database |

## NOTES

- This tool must typically be run as root user
- The tool creates a backup of the existing database before modifications
- Recovery logs are written to the system log

## SEE ALSO

- [widget-mgr](widget-mgr.md) - Widget manager command-line tool
- [pkgcmd](pkgcmd.md) - Package manager command-line tool
- [pkg_recovery](pkg_recovery.md) - Package recovery service

## PACKAGE

`widget-manager` (Tizen package name)
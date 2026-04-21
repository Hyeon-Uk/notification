# pkg-db-recovery

## NAME

pkg-db-recovery - Check and recover corrupted Tizen package manager database

## SYNOPSIS

```
pkg-db-recovery
```

## DESCRIPTION

`pkg-db-recovery` is a Tizen-specific tool for checking integrity and recovering corrupted package manager databases. It runs automatically without any command-line options and performs the following operations:

1. Checks integrity of global package databases (parser DB and cert DB)
2. Checks database schema validity
3. Scans all user databases for corruption
4. Automatically recovers corrupted databases by rebuilding them

This tool is typically invoked during system boot or manually when package database issues are suspected. It must be run as root user.

## USAGE

The tool accepts no command-line arguments. Simply execute:

```bash
pkg-db-recovery
```

## RECOVERY PROCESS

The tool performs the following checks and recovery actions:

1. **Certificate database check**: Validates `/opt/usr/dbspace/.pkgmgr_cert.db`
2. **Parser database check**: Validates `/opt/usr/dbspace/.pkgmgr_parser.db`
3. **Schema validation**: Checks database schema against expected structure
4. **User database scan**: Scans all user databases under `/opt/usr/dbspace/user/<uid>/`
5. **Automatic recovery**: If corruption is detected, rebuilds the database using `pkg-db-creator` and `pkg_initdb`

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/dbspace/.pkgmgr_parser.db` | Package parser database |
| `/opt/usr/dbspace/.pkgmgr_cert.db` | Package certificate database |
| `/opt/usr/dbspace/user/<uid>/.pkgmgr_parser.db` | User-specific package database |
| `/opt/usr/dbspace/.need_pkg_recovery_<uid>` | Recovery flag file |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success (databases are valid or recovered) |
| -1 | Error (permission denied or recovery failed) |

## NOTES

- This tool must be run as root user (uid 0)
- Regular users (uid > 5000) cannot execute this tool
- The tool creates recovery flag files during the recovery process
- Recovery is performed by calling `pkg-db-creator` and `pkg_initdb` internally

## SEE ALSO

- [pkg_initdb](pkg_initdb.md) - Initialize package manager database
- [pkg_recovery](pkg_recovery.md) - Package recovery service
- [pkg-db-creator](pkg-db-creator.md) - Create package database for user

## PACKAGE

`pkgmgr-info` (Tizen package name)
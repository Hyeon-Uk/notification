# pkg-db-creator

## NAME

pkg-db-creator - Create Tizen package manager database for a specific user

## SYNOPSIS

```
pkg-db-creator <uid>
```

## DESCRIPTION

`pkg-db-creator` is a Tizen-specific tool for creating package manager database entries for a specific user. It removes existing database files and creates new, empty databases with the proper schema.

This tool is typically called internally by `pkg-db-recovery` during the recovery process. It must be run as root user.

## USAGE

The tool accepts a single argument - the user ID (uid) for which to create the database:

| Argument | Description |
|----------|-------------|
| `<uid>` | User ID for which to create the package database |

## EXAMPLES

### Create database for root user (global)

```bash
pkg-db-creator 0
```

### Create database for specific user

```bash
pkg-db-creator 5001
```

### Create database for global app user

```bash
pkg-db-creator $(tzplatform-getuid TZ_SYS_GLOBALAPP_USER)
```

## PROCESS

The tool performs the following operations:

1. **Remove existing database**: Removes existing parser and certificate database files
2. **Remove journal files**: Cleans up any SQLite journal files
3. **Create new database**: Initializes a new package manager database with proper schema

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/dbspace/.pkgmgr_parser.db` | Package parser database (global) |
| `/opt/usr/dbspace/.pkgmgr_cert.db` | Package certificate database (global) |
| `/opt/usr/dbspace/user/<uid>/.pkgmgr_parser.db` | User-specific package database |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| -1 | Error (permission denied or database creation failed) |

## NOTES

- This tool must be run as root user (uid 0)
- The tool removes existing databases before creating new ones
- For root user (uid 0) and global user, both parser and cert databases are created
- For regular users, only the parser database is created

## SEE ALSO

- [pkg-db-recovery](pkg-db-recovery.md) - Package database recovery tool
- [pkg_initdb](pkg_initdb.md) - Initialize package manager database
- [pkg_recovery](pkg_recovery.md) - Package recovery service

## PACKAGE

`pkgmgr-info` (Tizen package name)
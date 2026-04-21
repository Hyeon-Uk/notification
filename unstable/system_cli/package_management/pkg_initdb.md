# pkg_initdb

## NAME

pkg_initdb - Initialize Tizen package manager database with preloaded packages

## SYNOPSIS

```
pkg_initdb [options]
```

## DESCRIPTION

`pkg_initdb` is a Tizen-specific tool for initializing the package manager database. It creates the necessary database structures and registers preloaded packages from the filesystem into the package database.

This tool is typically used during system initialization or when the package database needs to be rebuilt. It must be run as root user.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-u [ --uid ] arg` | User ID (default: 0) |
| `--partial-rw` | Process RW partition only |
| `--ro` | Process readonly packages only |
| `--rw` | Process RW packages only |
| `--keep-db` | Keep current database (do not recreate) |
| `-h [ --help ]` | Display help message |
| `--recover-db` | Register package database only (no package installation) |

## EXAMPLES

### Initialize database for root user

```bash
pkg_initdb
```

### Initialize database for specific user

```bash
pkg_initdb --uid 5001
```

### Initialize with recovery mode

```bash
pkg_initdb --recover-db --uid 5001 --keep-db
```

### Process readonly packages only

```bash
pkg_initdb --ro
```

### Process RW packages only

```bash
pkg_initdb --rw
```

### Process RW partition only

```bash
pkg_initdb --partial-rw
```

## OPTION CONFLICTS

The following option combinations are not allowed:

- `--ro` and `--rw` (conflicting options)
- `--rw` and `--partial-rw` (conflicting options)
- `--recover-db` and `--partial-rw` (conflicting options)
- `--ro`, `--rw`, or `--partial-rw` with non-global user uid

## PROCESS

The tool performs the following operations:

1. **Database handling**: Creates or keeps database based on options
2. **Directory scanning**: Scans package directories for preloaded packages
3. **Backend execution**: Registers packages using appropriate backend (tpk-backend, wgt-backend, etc.)
4. **Validation**: Validates package integrity during registration

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/dbspace/.pkgmgr.db` | Package manager database |
| `/usr/bin/pkg_initdb` | Executable path |
| `/etc/package-manager/backend/` | Backend scripts directory |
| `/tmp/.preload_install_error` | Error flag file |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| -1 | Error (permission denied, invalid options, or initialization failed) |

## NOTES

- This tool must be run as root user
- The `--uid` option accepts both root (0) and regular user IDs
- Options `--ro`, `--rw`, and `--partial-rw` only affect global user
- The tool creates error flag file on failure

## SEE ALSO

- [pkg-db-recovery](pkg-db-recovery.md) - Package database recovery tool
- [pkg_recovery](pkg_recovery.md) - Package recovery service
- [pkg-db-creator](pkg-db-creator.md) - Create package database for user

## PACKAGE

`app-installers` (Tizen package name)
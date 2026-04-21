# pkg_recovery

## NAME

pkg_recovery - Tizen package recovery service for incomplete installations

## SYNOPSIS

```
pkg_recovery
```

## DESCRIPTION

`pkg_recovery` is a Tizen-specific tool for recovering packages from incomplete or interrupted installations. It scans for recovery files left by the package manager and processes them to complete or rollback pending package operations.

This tool runs automatically without any command-line options and is typically invoked during system boot to recover from interrupted package installations or removals.

## USAGE

The tool accepts no command-line arguments. Simply execute:

```bash
pkg_recovery
```

## RECOVERY PROCESS

The tool performs the following operations:

1. **Search recovery files for global user**: Scans for recovery files belonging to the global user
2. **Search recovery files for regular users**: Iterates through all regular users and scans their recovery files
3. **Process recovery entries**: For each recovery file found, executes the appropriate backend to complete or rollback the operation
4. **Cleanup**: Removes processed recovery files

## RECOVERY FILE FORMAT

Recovery files contain paths to package files or special commands:

- **Package paths**: Each line contains a path to a package file that needs to be processed
- **cleanup**: A special command indicating that a cleanup operation should be performed

## BACKEND EXECUTION

The tool uses backend executables located in `/etc/package-manager/backend/`:

| Backend Type | Description |
|--------------|-------------|
| `tpk-backend` | Tizen Package (TPK) handler |
| `wgt-backend` | Widget Package handler |
| `unified-backend` | Unified package handler |

For unified backend, the tool reads the recovery file and parses its contents for additional arguments.

## FILES

| File/Directory | Description |
|----------------|-------------|
| `/etc/package-manager/backend/` | Backend executables directory |
| `/var/lib/package-manager/recovery/` | Recovery files directory |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success (all recovery files processed) |
| -1 | Error (exception occurred during processing) |

## NOTES

- Individual recovery failures are logged but do not stop the overall recovery process
- The tool processes both global user and regular user recovery files
- Recovery files are removed after successful processing
- Backend execution failures are logged as errors

## SEE ALSO

- [pkg-db-recovery](pkg-db-recovery.md) - Package database recovery tool
- [pkg_initdb](pkg_initdb.md) - Initialize package manager database
- [pkg-db-creator](pkg-db-creator.md) - Create package database for user

## PACKAGE

`app-installers` (Tizen package name)
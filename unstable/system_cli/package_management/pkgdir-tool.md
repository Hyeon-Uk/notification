# pkgdir-tool

## NAME

pkgdir-tool - D-Bus service for managing package directories (deprecated)

## SYNOPSIS

```
pkgdir-tool
```

## DESCRIPTION

`pkgdir-tool` is a D-Bus system service that provides functionality for managing external package directories. The service runs as a daemon and listens for D-Bus method calls.

**Note:** This service is deprecated. External read-write directory functionality is no longer used, and this tool will be removed in future versions of Tizen.

## D-BUS INTERFACE

### Service Name

```
org.tizen.pkgdir_tool
```

### Object Path

```
/org/tizen/pkgdir_tool
```

### Methods

| Method | Parameters | Returns | Description |
|--------|------------|---------|-------------|
| `CreateExternalDirsForAllPkgs` | None | `(b) result` | Create external directories for all installed packages (deprecated/disabled) |

## SERVICE BEHAVIOR

The service operates as follows:

1. Registers on the system D-Bus bus with service name `org.tizen.pkgdir_tool`
2. Listens for incoming method calls
3. Automatically terminates after 5 seconds of inactivity
4. The timeout is renewed after each method call

## SYSTEMD SERVICE

The service is typically started via systemd:

```bash
systemctl start pkgdir-tool
```

Service file location: `/usr/lib/systemd/system/pkgdir-tool.service`

## DEPRECATION NOTICE

This tool is deprecated and will be removed in future Tizen releases:

- External read-write directories are no longer used by the package manager
- The `CreateExternalDirsForAllPkgs` method is disabled in the code
- The service returns `true` for all method calls without performing any action

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Service exited normally (timeout) |
| -1 | Initialization failed |

## FILES

| File | Description |
|------|-------------|
| `/usr/lib/systemd/system/pkgdir-tool.service` | Systemd service unit file |

## SEE ALSO

- [pkgcmd](pkgcmd.md) - Package manager command-line tool
- [pkginfo](pkginfo.md) - Query package information

## PACKAGE

`app-installers` (Tizen package name)
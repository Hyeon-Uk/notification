# pkginfo

## NAME

pkginfo - Query package information (API-based utility)

## SYNOPSIS

```
pkginfo [options] <package-id>
```

## DESCRIPTION

`pkginfo` is a utility for querying information about installed Tizen packages. It retrieves package metadata from the package manager database and displays details such as package version, type, installation path, and associated applications.

**Note:** This tool may not be available as a standalone command-line utility on all Tizen systems. Package information queries are primarily performed through the `pkgmgr-info` API library.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-l` | List all installed packages |
| `-t <type>` | Filter by package type (tpk, wgt) |
| `-n <pkgid>` | Specify package ID to query |
| `-v` | Verbose output |
| `-h` | Print help message |

## USAGE EXAMPLES

### List All Packages

```bash
pkginfo -l
```

### Query Specific Package

```bash
pkginfo -n org.example.hello
```

### Filter by Package Type

```bash
pkginfo -l -t tpk
```

## OUTPUT FIELDS

When displaying package information, the following fields may be shown:

| Field | Description |
|-------|-------------|
| `Package ID` | Unique identifier for the package |
| `Type` | Package type (tpk, wgt) |
| `Version` | Package version string |
| `Label` | Package display name |
| `Storage` | Installation storage (internal/external) |
| `Installed Size` | Total installed size in bytes |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 1 | Package not found |
| 2 | Invalid arguments |

## ALTERNATIVE METHODS

If `pkginfo` command is not available, use one of these alternatives:

### Using pkgcmd

```bash
pkgcmd -l           # List all packages
pkgcmd -s -n <pkgid> # Show package info
```

### Using pkgmgr-info API

Package information can be queried programmatically using the `pkgmgr-info` library functions:

- `pkgmgrinfo_pkginfo_get_pkginfo()` - Get package info handle
- `pkgmgrinfo_pkginfo_get_version()` - Get package version
- `pkgmgrinfo_pkginfo_get_type()` - Get package type
- `pkgmgrinfo_pkginfo_get_label()` - Get package label

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/dbspace/.pkgmgr_parser.db` | Package manager database |
| `/opt/usr/apps/<pkgid>/` | Package installation directory |

## SEE ALSO

- [pkgcmd](pkgcmd.md) - Package manager command-line tool
- [pkgmgr-info](pkgmgr-info) - Package manager info library (API)

## PACKAGE

`pkgmgr-info` (Tizen package name)
# tpk-backend

## NAME

tpk-backend - TPK package installation backend service

## SYNOPSIS

```
tpk-backend
```

## DESCRIPTION

`tpk-backend` is a backend service/library for handling Tizen Package (TPK) installation, uninstallation, and management operations. It serves as the internal handler for TPK-specific package operations within the Tizen package manager framework.

This is not a user-facing command-line tool. It operates as a backend plugin that is invoked by the package manager daemon (`pkgmgrd`) when processing TPK packages.

## FUNCTIONALITY

The TPK backend provides the following operations:

- Package installation from TPK files
- Package uninstallation
- Package upgrade/reinstallation
- Manifest parsing and processing
- File extraction and placement
- Database registration
- Security context setup

## INTEGRATION

The TPK backend integrates with:

- Package Manager Daemon (`pkgmgrd`)
- Package Manager Client Library (`libpkgmgr`)
- Security Manager for privilege handling
- Application Launcher for app registration

## FILES

| File | Description |
|------|-------------|
| `/usr/lib/libtpk-backend.so` | TPK backend library |
| `/usr/share/package-manager/plugins/` | Backend plugin directory |

## SEE ALSO

- [pkgcmd](pkgcmd.md) - Package manager command-line tool
- [wgt-backend](wgt-backend.md) - Widget package backend
- [pkgmgrd](pkgmgrd) - Package manager daemon

## PACKAGE

`tpk-backend` (Tizen package name)
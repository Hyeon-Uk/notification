# wgt-backend

## NAME

wgt-backend - Widget (WGT) package installation backend service

## SYNOPSIS

```
wgt-backend
```

## DESCRIPTION

`wgt-backend` is a backend service/library for handling Widget Package (WGT) installation, uninstallation, and management operations. It serves as the internal handler for WGT-specific package operations within the Tizen package manager framework.

This is not a user-facing command-line tool. It operates as a backend plugin that is invoked by the package manager daemon (`pkgmgrd`) when processing WGT widget packages.

## FUNCTIONALITY

The WGT backend provides the following operations:

- Widget installation from WGT files
- Widget uninstallation
- Widget upgrade/reinstallation
- config.xml manifest parsing
- Web application resource extraction
- Database registration
- Web runtime integration

## WIDGET PACKAGE STRUCTURE

A WGT package contains:

- `config.xml` - Widget configuration file
- `index.html` - Main HTML file
- Web resources (CSS, JavaScript, images)
- Icon files
- Localization files

## INTEGRATION

The WGT backend integrates with:

- Package Manager Daemon (`pkgmgrd`)
- Web Runtime (WRT)
- Web Engine (Chromium-based)
- Widget Manager Service

## FILES

| File | Description |
|------|-------------|
| `/usr/lib/libwgt-backend.so` | WGT backend library |
| `/usr/share/package-manager/plugins/` | Backend plugin directory |

## SEE ALSO

- [pkgcmd](pkgcmd.md) - Package manager command-line tool
- [tpk-backend](tpk-backend.md) - TPK package backend
- [widget-mgr](widget-mgr.md) - Widget manager

## PACKAGE

`wgt-backend` (Tizen package name)
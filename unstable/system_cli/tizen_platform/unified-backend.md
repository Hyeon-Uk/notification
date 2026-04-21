# unified-backend

## NAME

unified-backend - Tizen unified package installation backend

## SYNOPSIS

```
unified-backend [OPTIONS...] [PACKAGE_FILE]
```

## DESCRIPTION

unified-backend is a unified package installation backend tool for the Tizen platform. It provides functionality to install, remove, and update various package formats such as TPK and WGT. It operates as a backend for the package manager (pkgmgr) framework and handles installation requests.

**Supported Package Formats:**
- TPK (Tizen Package)
- WGT (Widget Package)

## OPTIONS

This tool is invoked through the package manager framework and does not provide direct user options. Arguments are passed by pkgmgr.

## USAGE

unified-backend is typically called by the package manager daemon. It can also be executed directly:

Install a package:
```
$ unified-backend -i /path/to/package.tpk
```

Remove a package:
```
$ unified-backend -u package-id
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred |

## NOTES

- This tool is designed as a backend for the package manager framework
- It is recommended to use pkgcmd or pkgmgr API rather than executing directly
- Installation logs are recorded in the system log

## SEE ALSO

- pkgcmd(1)
- pkgmgr(8)
- tpk-backend(1)
- wgt-backend(1)

## PACKAGE

vd-unified-backend
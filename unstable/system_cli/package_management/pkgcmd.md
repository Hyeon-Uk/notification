# pkgcmd

## NAME

pkgcmd - Tizen package manager command-line tool for package installation, uninstallation, and management

## SYNOPSIS

```
pkgcmd [options]
```

## DESCRIPTION

`pkgcmd` is the primary command-line tool for managing Tizen packages. It provides comprehensive functionality for installing, uninstalling, reinstalling, and managing Tizen application packages (TPK, WGT) and their associated resources.

The tool supports both single-package and multiple-package operations, making it suitable for batch processing and system maintenance tasks.

## OPTIONS

### Package Operations

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-i` | `--install` | Install a package |
| `-u` | `--uninstall` | Uninstall a package |
| `-r` | `--reinstall` | Reinstall a package |
| `-w` | `--mount-install` | Mount install a package (without copying to internal storage) |
| `-c` | `--clear` | Clear user data for a package |
| | `--clear-all` | Clear user data for all packages or packages of a specific type |
| | `--clear-cache` | Clear cache data for a package |
| `-m` | `--move` | Move package between storage locations |
| `-g` | `--getsize` | Get size of a given package |
| `-x` | `--getsizeinfo` | Get detailed size information of a package |
| `-l` | `--list` | Display list of installed packages available for the current user |
| `-s` | `--show` | Show detailed package information |
| `-a` | `--app-path` | Show application installation path |
| `-C` | `--check` | Check if applications belonging to a package are running |
| `-k` | `--kill` | Terminate applications belonging to a package |

### Activation Operations

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-A` | `--activate` | Enable package or application |
| `-D` | `--deactivate` | Disable package or application |
| `-L <label>` | `--activate-with-label <label>` | Activate with a specified label |

### Update Information Operations

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-R <type>` | `--reg-updateinfo <type>` | Register update information (0: NONE, 1: FORCE, 2: OPTIONAL) |
| `-U <mode>` | `--unreg-updateinfo <mode>` | Unregister update information (0: ONE, 1: ALL) |
| `-E` | `--uninstall-res` | Request uninstall resource |

### Package Identification

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-t <type>` | `--package-type <type>` | Specify package type (tpk, wgt) |
| `-n <name>` | `--package-name <name>` | Specify package name/ID |
| `-p <path>` | `--package-path <path>` | Specify package file path |
| `-d <path>` | `--descriptor <path>` | Specify descriptor path |

### Move Operations

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-T <type>` | `--move-type <type>` | Specify move type (0: internal, 1: external, 2: extended) |
| `-T <type>` | `--getsize-type <type>` | Specify getsize type (0: total size, 1: data size) |

### TEP Package Options

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-e <path>` | `--tep-path <path>` | Specify TEP package path |
| `-M <mode>` | `--tep-move <mode>` | TEP move/copy mode (0: copy, 1: move) |

### Delta Package Options

| Option | Long Option | Description |
|--------|-------------|-------------|
| `-X <pkg>` | `--old_pkg <pkg>` | Specify old package for delta creation |
| `-Y <pkg>` | `--new_pkg <pkg>` | Specify new package for delta creation |
| `-Z <pkg>` | `--delta_pkg <pkg>` | Specify output delta package path |

### User and Mode Options

| Option | Long Option | Description |
|--------|-------------|-------------|
| | `--global` | Global mode (requires privileged user) |
| | `--uid <uid>` | Specify target user ID |
| `-G` | `--debug-mode` | Enable debug mode for SDK |
| `-S` | `--skip-optimization` | Skip optimization during installation |
| `-h` | `--help` | Print help message |

## USAGE EXAMPLES

### Install a Package

```bash
# Install a TPK package
pkgcmd -i -t tpk -p /tmp/org.example.hello-1.0.0-arm.tpk

# Install with debug mode
pkgcmd -i -t tpk -p /tmp/org.example.hello-1.0.0-arm.tpk -G

# Install with TEP package
pkgcmd -i -t tpk -p /tmp/org.example.hello-1.0.0-arm.tpk -e /tmp/example.tep
```

### Uninstall a Package

```bash
# Uninstall a package
pkgcmd -u -n org.example.hello

# Uninstall globally
pkgcmd -u -n org.example.hello --global
```

### Reinstall a Package

```bash
pkgcmd -r -t tpk -n org.example.hello
```

### Mount Install

```bash
pkgcmd -w -t tpk -p /tmp/org.example.hello-1.0.0-arm.tpk
```

### List Packages

```bash
# List all packages
pkgcmd -l

# List packages of specific type
pkgcmd -l -t tpk
```

### Clear Data

```bash
# Clear user data for a package
pkgcmd -c -t tpk -n org.example.hello

# Clear all packages of a type
pkgcmd --clear-all -t wgt

# Clear cache data
pkgcmd --clear-cache -n org.example.hello
```

### Get Package Information

```bash
# Get package size
pkgcmd -g -T 0 -n org.example.hello

# Get detailed size info
pkgcmd -x -n org.example.hello

# Show app installation path
pkgcmd -a
pkgcmd -a -t tpk -n org.example.hello
```

### Application Management

```bash
# Check if app is running
pkgcmd -C -n org.example.hello

# Kill running app
pkgcmd -k -n org.example.hello

# Deactivate a package
pkgcmd -D -t tpk -n org.example.hellopkg

# Deactivate an app globally
pkgcmd -D -t app -n org.example.helloapp --global

# Activate with label
pkgcmd -A -L "My App" -t app -n org.example.helloapp
```

### Move Package

```bash
# Move to external storage
pkgcmd -m -t tpk -T 1 -n org.example.hello
```

### Update Information Management

```bash
# Register update info (force update)
pkgcmd -R 1 -n org.example.hellopkg 1.0

# Unregister update info for one package
pkgcmd -U 0 -n org.example.hellopkg

# Unregister update info for all packages
pkgcmd -U 1
```

### Create Delta Package

```bash
pkgcmd -X /tmp/old.tpk -Y /tmp/new.tpk -Z /tmp/delta_pkg
```

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| Non-zero | Error occurred (see error message for details) |

### Common Error Codes

| Error Code | Description |
|------------|-------------|
| `PKGMGR_INSTALLER_ERRCODE_PACKAGE_NOT_FOUND` | Package not found |
| `PKGMGR_INSTALLER_ERRCODE_PARSE_ERROR` | Package parsing error |
| `PKGMGR_INSTALLER_ERRCODE_SECURITY_ERROR` | Security/permission error |
| `PKGMGR_INSTALLER_ERRCODE_OUT_OF_SPACE` | Insufficient storage space |
| `PKGMGR_INSTALLER_ERRCODE_INVALID_VALUE` | Invalid argument value |
| `PKGMGR_INSTALLER_ERRCODE_SIGNATURE_ERROR` | Signature verification failed |
| `PKGMGR_INSTALLER_ERRCODE_PRIVILEGE_ERROR` | Privilege level error |

## NOTES

- Most operations require appropriate permissions (root or package manager user)
- The `--global` option affects system-wide packages and requires elevated privileges
- Multiple packages can be specified for install/uninstall operations by listing them after the options
- Delta package creation requires root user privileges
- Package types: `tpk` (Tizen Package), `wgt` (Widget), `app` (Application)

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/apps/<pkgid>/` | Application installation directory |
| `/opt/usr/dbspace/.pkgmgr_parser.db` | Package manager database |
| `/etc/package-manager/pkgmgr-unzip-pkg.sh` | Package unzip script |
| `/etc/package-manager/pkgmgr-create-delta.sh` | Delta package creation script |

## SEE ALSO

- [pkginfo](pkginfo.md) - Query package information
- [pkgdir-tool](pkgdir-tool.md) - Package directory management tool
- [widget-mgr](widget-mgr.md) - Widget manager

## PACKAGE

`pkgmgr-tool` (Tizen package name)
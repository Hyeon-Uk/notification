# TIZEN-PACKAGE-MANAGER-CLI(1)

## NAME
**tizen-package-manager-cli** - Query installed Tizen package metadata and
package-manager derived state.

## SYNOPSIS
**tizen-package-manager-cli** *COMMAND* [*OPTIONS*]  
**tizen-package-manager-cli** **-h** | **--help** | **help**  
**tizen-package-manager-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-package-manager-cli** provides command-line access to the public
package manager APIs from `package_manager.h` and `package_info.h`.

The tool focuses on finite package queries that are practical in a shell:
- listing installed packages
- retrieving detailed package metadata
- resolving `app_id -> package_id`
- checking preload and permission classification for an app
- retrieving package size counters
- requesting per-package cache cleanup

Most query commands require the public privilege
`http://tizen.org/privilege/packagemanager.info`.
`clear-cache` requires
`http://tizen.org/privilege/packagemanager.clearcache`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list** | None | `packagemanager.info` | Lists installed package IDs with label, version, and package type. |
| **info** | `--package-id <ID>` | `packagemanager.info` | Prints package metadata including main app ID, icon path, storage, root path, and boolean flags. |
| **lookup** | `--app-id <ID>` | `packagemanager.info` | Resolves the package ID that owns an application ID. |
| **preload-by-app** | `--app-id <ID>` | `packagemanager.info` | Reports whether the package that owns the application is a preload package. |
| **permission-type** | `--app-id <ID>` | `packagemanager.info` | Prints the package permission class for the application ID. |
| **size-info** | `--package-id <ID>` | `packagemanager.info` | Prints package data, cache, app, and external size counters returned by `package_manager_get_package_size_info()`. |
| **clear-cache** | `--package-id <ID>` | `packagemanager.clearcache` | Requests asynchronous cache cleanup for the package. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, privilege failure, API failure, or callback timeout.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-package-manager-cli`

## AUTHOR
Part of the **Tizen Tools** project.

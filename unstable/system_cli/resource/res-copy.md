# RES-COPY

**NAME**

res-copy - resource copy utility for Tizen package management

**SYNOPSIS**

```
res-copy [OPTIONS]
```

**DESCRIPTION**

`res-copy` is a command-line utility used internally by the Tizen package manager to handle resource files during package installation, uninstallation, and maintenance operations. It copies, removes, and manages shared resources for installed packages.

This tool is typically invoked by the package manager daemon during package operations and is not intended for direct user interaction.

**OPTIONS**

| Option | Long Form | Argument | Description |
|--------|-----------|----------|-------------|
| `-u` | `--uid` | UID | User ID for the operation (default: 0) |
| `-k` | `--session-id` | SESSION_ID | Session ID for tracking the request |
| `-p` | `--path` | SRC [DST] | Source and destination path pair for copy operations |
| `-r` | `--remove` | PKGID | Remove shared resource for the specified package |
| `-d` | `--delete` | PKGID | Delete shared resource for the specified package (uninstall) |
| `-c` | `--copy` | PKGID | Copy resource for the specified package |
| `-D` | `--createdir` | PKGID | Create directories for the specified package |
| `-h` | `--help` | - | Display help message |

**OPERATION TYPES**

The tool supports several operation modes based on the selected option:

1. **Copy Mode** (`-c`): Copies resources from source to destination paths during package installation.
2. **Remove Mode** (`-r`): Removes shared resources for a package.
3. **Delete Mode** (`-d`): Deletes resources during package uninstallation.
4. **Create Directory Mode** (`-D`): Creates necessary directories for package resources.

**EXAMPLES**

```
# Copy resources for a package with UID 5000
res-copy -u 5000 -c org.example.app -p /source/path /dest/path

# Create directories for a package
res-copy -u 5000 -D org.example.app

# Remove shared resources for a package
res-copy -u 5000 -r org.example.app

# Delete resources during uninstallation
res-copy -u 5000 -d org.example.app
```

**SECURITY**

The `res-copy` binary requires special Linux capabilities to perform its operations:

| Capability | Purpose |
|------------|---------|
| `cap_chown` | Change copied file's ownership |
| `cap_dac_override` | Bypass file read/write/execute permission checks |
| `cap_fowner` | Bypass permission checks on operations requiring file owner |
| `cap_mac_override` | Abort app directories creation/deletion |

These capabilities are set during package installation.

**EXIT STATUS**

| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Failure (invalid parameters, initialization error, or execution error) |

**SEE ALSO**

pkgcmd(1), pkginfo(1), rsc-slice(1)

**PACKAGE**

platform/core/appfw/pkgmgr-tool
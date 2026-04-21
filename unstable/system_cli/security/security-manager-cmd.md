# SECURITY-MANAGER-CMD

**NAME**

security-manager-cmd - command-line tool for security manager offline operations

**SYNOPSIS**

```
security-manager-cmd [COMMAND] [OPTIONS]
```

**DESCRIPTION**

`security-manager-cmd` is a command-line utility for managing Tizen security policies in offline mode. It provides functionality to install/uninstall applications, register paths, manage users, and control privileges for the security manager database.

**COMMANDS**

| Command | Description |
|---------|-------------|
| `-n, --manage-apps` | Add or remove an application (arg: a/add or r/remove) |
| `-r, --register-paths` | Register paths for installed application |
| `-m, --manage-users` | Add or remove user (arg: a/add or r/remove) |
| `-o, --manage-privilege` | Allow or deny privilege (arg: a/allow or d/deny) |
| `-b, --backup` | Create backup of the security database |
| `-i, --install` | Install an application (deprecated, use manage-apps) |
| `-h, --help` | Display help message |

**APPLICATION OPTIONS**

| Option | Argument | Description |
|--------|----------|-------------|
| `-a, --app` | NAME | Application name (required) |
| `-g, --pkg` | NAME | Package name (required) |
| `-p, --path` | PATH TYPE | Path for SMACK labels (may occur multiple times) |
| `-s, --privilege` | PRIV | Privilege for the application (may occur multiple times) |
| `-u, --uid` | UID | User identifier number (required) |
| `-t, --tizen` | VERSION | Target Tizen version (e.g., 2.4, 3.0) |
| `-c, --author-id` | ID | Author's unique identifier |
| `--install-type` | TYPE | Installation type: local, global, preloaded |
| `--hybrid` | - | Sets hybrid flag |
| `--pkg-privilege-level` | LEVEL | Package privilege level: public, partner, platform |
| `--pkg-type` | TYPE | Package type: wrt, core, metadata |

**PATH TYPES**

| Type | Description |
|------|-------------|
| `rw` | Read-write access |
| `ro` | Read-only access |
| `public_ro` | Public read-only access |
| `rw_others_ro` | Owner read-write, others read-only |
| `trusted_rw` | Trusted read-write (requires author-id) |

**USER TYPES**

| Type | Description |
|------|-------------|
| `system` | System user |
| `admin` | Administrator user |
| `guest` | Guest user |
| `normal` | Normal user (default) |

**EXAMPLES**

```
# Install an application
security-manager-cmd -n add -a MyApp -g org.example.myapp -u 5000 \
  -p /opt/usr/apps/MyApp rw -s http://tizen.org/privilege/location

# Remove an application
security-manager-cmd -n remove -a MyApp -g org.example.myapp -u 5000

# Register paths for a package
security-manager-cmd -r -g org.example.app -u 5000 -p /data/app rw

# Add a user
security-manager-cmd -m add -u 5001 -t normal

# Remove a user
security-manager-cmd -m remove -u 5001

# Allow a privilege
security-manager-cmd -o allow -s http://tizen.org/privilege/camera -a MyApp -u 5000

# Create database backup
security-manager-cmd -b
```

**EXIT STATUS**

| Value | Description |
|-------|-------------|
| 0 | Success |
| 1 | Failure |

**SEE ALSO**

security-manager(8), chsmack(1)

**PACKAGE**

platform/core/security/security-manager
# ISU

**NAME**

isu - install/upgrade utility for Tizen

**SYNOPSIS**

```
isu [OPTIONS] PACKAGE
```

**DESCRIPTION**

`isu` is a command-line utility for installing and upgrading packages in Tizen. It provides secure package installation with signature verification and dependency resolution.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-i, --install` | Install package |
| `-U, --upgrade` | Upgrade package |
| `-f, --force` | Force installation |
| `-v, --verbose` | Verbose output |

**EXAMPLES**

```
# Install a package
isu -i package.rpm

# Upgrade a package
isu -U package.rpm

# Force installation
isu -f -i package.rpm
```

**SEE ALSO**

isu-sandbox(1), pkgcmd(1)

**PACKAGE**

Tizen platform tool
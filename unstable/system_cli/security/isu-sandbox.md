# ISU-SANDBOX

**NAME**

isu-sandbox - sandboxed package installation utility

**SYNOPSIS**

```
isu-sandbox [OPTIONS] PACKAGE
```

**DESCRIPTION**

`isu-sandbox` is a sandboxed version of the isu package installer. It runs package installation in an isolated environment using namespaces and cgroups for enhanced security during the installation process.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-i, --install` | Install package |
| `-f, --force` | Force installation |

**EXAMPLES**

```
# Install package in sandbox
isu-sandbox -i package.rpm
```

**SEE ALSO**

isu(1), nsjail(1)

**PACKAGE**

Tizen platform tool
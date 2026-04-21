# SMACKCTL

**NAME**

smackctl - control SMACK configuration

**SYNOPSIS**

```
smackctl [COMMAND] [OPTIONS]
```

**DESCRIPTION**

`smackctl` is a command-line utility for managing SMACK (Simplified Mandatory Access Control Kernel) configuration. It allows administrators to load, modify, and control SMACK rules and settings in the running kernel.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h` | Display help message |
| `-v` | Verbose output |

**EXAMPLES**

```
# Display current SMACK configuration
smackctl

# Load SMACK rules from file
smackctl load /etc/smack/accesses.d/rules

# Show SMACK status
smackctl status
```

**SEE ALSO**

chsmack(1), smackload(1), smackaccess(1)

**PACKAGE**

External package: smack-utils
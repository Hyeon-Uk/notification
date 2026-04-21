# SMACKLOAD

**NAME**

smackload - load SMACK rules into the kernel

**SYNOPSIS**

```
smackload [OPTIONS] [RULE_FILE]
```

**DESCRIPTION**

`smackload` is a command-line utility that loads SMACK (Simplified Mandatory Access Control Kernel) access rules into the running kernel. Rules are typically read from files in `/etc/smack/accesses.d/` or from standard input.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h` | Display help message |
| `-v` | Verbose output |
| `-d` | Directory containing rule files |

**RULE FORMAT**

Each rule line contains:
```
SUBJECT OBJECT MODE
```

Where:
- SUBJECT: The SMACK label of the subject (process)
- OBJECT: The SMACK label of the object (file/resource)
- MODE: Access mode characters: `r` (read), `w` (write), `x` (execute), `a` (append)

**EXAMPLES**

```
# Load rules from standard input
echo "User Home rw" | smackload

# Load rules from a file
smackload /etc/smack/accesses.d/myrules

# Load all rules from default directory
smackload -d /etc/smack/accesses.d/
```

**EXIT STATUS**

| Value | Description |
|-------|-------------|
| 0 | Success |
| 1 | Failure |

**SEE ALSO**

chsmack(1), smackctl(1), smackaccess(1)

**PACKAGE**

External package: smack-utils
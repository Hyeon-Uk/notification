# CHSMACK

**NAME**

chsmack - change or display SMACK extended attributes

**SYNOPSIS**

```
chsmack [OPTIONS] FILE...
chsmack -r DIRECTORY
```

**DESCRIPTION**

`chsmack` is a command-line utility for managing SMACK (Simplified Mandatory Access Control Kernel) extended attributes on files and directories. SMACK is a Linux kernel security module that implements mandatory access control (MAC) using labels attached to objects.

This tool allows viewing and modifying SMACK security labels which control access permissions between processes and files in Tizen's security architecture.

**OPTIONS**

| Option | Argument | Description |
|--------|----------|-------------|
| `-a` | LABEL | Set the SMACK64 (access) attribute |
| `-e` | LABEL | Set the SMACK64EXEC (execute) attribute |
| `-m` | LABEL | Set the SMACK64MMAP (mmap) attribute |
| `-t` | LABEL | Set the SMACK64TRANSMUTE (transmute) attribute |
| `-r` | - | Recursive mode for directories |
| `-h` | - | Display help message |
| `-v` | - | Verbose output |

**SMACK ATTRIBUTES**

SMACK uses extended attributes to store security labels:

| Attribute | Extended Attribute Name | Description |
|-----------|------------------------|-------------|
| access | SMACK64 | Controls which processes can access this file |
| execute | SMACK64EXEC | Label applied to process when executing this file |
| mmap | SMACK64MMAP | Controls which processes can memory-map this file |
| transmute | SMACK64TRANSMUTE | Causes new files in directory to inherit parent's label |

**EXAMPLES**

```
# Display SMACK attributes of a file
chsmack /path/to/file

# Set access label on a file
chsmack -a "User::Home" /path/to/file

# Set execute label on an executable
chsmack -e "System::Shared" /usr/bin/myapp

# Recursively display SMACK attributes
chsmack -r /opt/usr/apps/

# Set multiple attributes
chsmack -a "App::Data" -t "App::Data" /opt/usr/apps/myapp/data/
```

**SECURITY LABELS**

Tizen uses SMACK labels to enforce security boundaries:
- Labels starting with underscore (`_`) are special (e.g., `_` for floor access)
- Labels contain alphanumeric characters, colons, and underscores
- Process labels determine what files they can access
- File labels determine which processes can access them

**EXIT STATUS**

| Value | Description |
|-------|-------------|
| 0 | Success |
| 1 | Failure (permission denied, file not found, etc.) |

**SEE ALSO**

smackctl(1), smackload(1), smackaccess(1), getfattr(1), setfattr(1)

**PACKAGE**

External package: smack-utils or similar SMACK utilities package
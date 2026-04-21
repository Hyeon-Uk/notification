# SMACKACCESS

**NAME**

smackaccess - check SMACK access permissions

**SYNOPSIS**

```
smackaccess SUBJECT OBJECT [MODE]
```

**DESCRIPTION**

`smackaccess` is a command-line utility that checks whether a SMACK subject (process) label has the specified access mode to an object (file) label according to the SMACK security rules. This tool is useful for testing and debugging SMACK access control policies.

**ARGUMENTS**

| Argument | Description |
|----------|-------------|
| SUBJECT | The SMACK label of the subject (process) |
| OBJECT | The SMACK label of the object (file/resource) |
| MODE | Optional access mode to check: `r` (read), `w` (write), `x` (execute), `a` (append). Default checks all modes. |

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h` | Display help message |
| `-v` | Verbose output |

**ACCESS MODES**

SMACK supports the following access modes:

| Mode | Character | Description |
|------|-----------|-------------|
| Read | `r` | Permission to read the object |
| Write | `w` | Permission to write/modify the object |
| Execute | `x` | Permission to execute the object |
| Append | `a` | Permission to append to the object |

**EXAMPLES**

```
# Check if System label can read User label
smackaccess System User r

# Check if App label can read/write Data label
smackaccess MyApp App::Data rw

# Check all access modes between labels
smackaccess System::Shared User::Home

# Check execute permission
smackaccess User App::Bin x
```

**EXIT STATUS**

| Value | Description |
|-------|-------------|
| 0 | Access is permitted |
| 1 | Access is denied |
| 2 | Invalid arguments |

**SEE ALSO**

chsmack(1), smackctl(1), smackload(1)

**PACKAGE**

External package: smack-utils
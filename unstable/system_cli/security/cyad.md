# CYAD

**NAME**

cyad - Cyanogenmod/Yonggo application daemon control

**SYNOPSIS**

```
cyad [COMMAND] [OPTIONS]
```

**DESCRIPTION**

`cyad` is a command-line utility for managing the CYA (Container Your Application) security framework in Tizen. It provides application container management and security policy enforcement.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-l, --list` | List containers |
| `-s, --status` | Show container status |

**EXAMPLES**

```
# List containers
cyad -l

# Show container status
cyad -s <container_name>
```

**PACKAGE**

Tizen platform tool
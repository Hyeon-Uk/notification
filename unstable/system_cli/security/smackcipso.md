# SMACKCIPSO

**NAME**

smackcipso - manage SMACK CIPSO mappings

**SYNOPSIS**

```
smackcipso [OPTIONS]
```

**DESCRIPTION**

`smackcipso` is a command-line utility for managing CIPSO (Commercial IP Security Option) mappings in SMACK. CIPSO allows SMACK labels to be transmitted across the network by encoding them in IP packet options.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h` | Display help message |
| `-l` | List current CIPSO mappings |
| `-a` | Add a CIPSO mapping |
| `-d` | Delete a CIPSO mapping |

**EXAMPLES**

```
# List CIPSO mappings
smackcipso -l

# Add a CIPSO mapping
smackcipso -a "System" 1
```

**SEE ALSO**

chsmack(1), smackctl(1)

**PACKAGE**

External package: smack-utils
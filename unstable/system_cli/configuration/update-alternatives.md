# update-alternatives - Maintain symbolic links for default commands

## NAME

**update-alternatives** - Maintain symbolic links determining default commands.

## SYNOPSIS

```bash
update-alternatives [options] <command>
```

## DESCRIPTION

**update-alternatives** creates, removes, maintains, and displays information about the symbolic links comprising the alternatives system. It manages symbolic links in `/usr/bin` that point to `/etc/alternatives/`, allowing multiple programs fulfilling the same function to be installed simultaneously.

## COMMANDS

| Command | Description |
|---------|-------------|
| `--install <link> <name> <path> <priority>` | Add a group of alternatives. |
| `--remove <name> <path>` | Remove `<path>` from `<name>` group. |
| `--remove-all <name>` | Remove all links for `<name>` group. |
| `--set <name> <path>` | Set `<path>` as alternative for `<name>`. |
| `--config <name>` | Interactive selection for `<name>`. |
| `--auto <name>` | Switch to automatic mode for `<name>`. |
| `--display <name>` | Display information about `<name>` group. |
| `--list <name>` | List all targets for `<name>`. |
| `--get-selections` | List all alternative selections. |
| `--set-selections` | Set alternatives from stdin. |

## OPTIONS

| Option | Description |
|--------|-------------|
| `--altdir <dir>` | Specify alternatives directory. Default: `/etc/alternatives` |
| `--admindir <dir>` | Specify administrative directory. Default: `/var/lib/dpkg/alternatives` |
| `--log <file>` | Specify log file. |
| `--verbose` | Verbose output. |
| `--quiet` | Quiet output. |
| `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The operation completed successfully. |
| `1` | Error. Invalid arguments or operation failed. |
| `2` | Error. Alternative not found. |

## EXAMPLES

### Interactive selection for java

```bash
$ update-alternatives --config java
```

### Set specific python version

```bash
$ update-alternatives --set python /usr/bin/python3.9
```

### Install new alternative

```bash
$ update-alternatives --install /usr/bin/java java /usr/lib/jvm/java-11/bin/java 1
```

### Display alternative information

```bash
$ update-alternatives --display java
```

### List all alternatives for a group

```bash
$ update-alternatives --list java
```

### Remove an alternative

```bash
$ update-alternatives --remove java /usr/lib/jvm/java-8/bin/java
```

### Switch to automatic mode

```bash
$ update-alternatives --auto java
```

## NOTES

- Scopes symbolic links in `/usr/bin` to point toward `/etc/alternatives/`.
- In manual mode, the link remains set until changed.
- In automatic mode, the highest priority alternative is selected.
- Requires root privileges for modifications.

## SEE ALSO

**dpkg**(1), **rpm**(8)

## PACKAGE

`dpkg`
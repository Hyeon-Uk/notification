# modprobe

**NAME**

modprobe - Add and remove modules from the Linux Kernel

**SYNOPSIS**

```bash
modprobe [OPTIONS] MODULE [ARGS...]
modprobe [OPTIONS] -r MODULE...
```

**DESCRIPTION**

`modprobe` intelligently adds or removes a module from the Linux kernel. Unlike insmod, modprobe automatically handles module dependencies, loading prerequisite modules as needed.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-a`, `--all` | Load all matching modules |
| `-c`, `--show-conf` | Show current configuration |
| `-d`, `--dirname=DIR` | Use DIR as module directory |
| `-n`, `--dry-run` | Don't actually insert/remove modules |
| `-i`, `--ignore-install` | Ignore install commands in configuration |
| `-q`, `--quiet` | Quiet mode |
| `-r`, `--remove` | Remove module (and its dependencies) |
| `-v`, `--verbose` | Verbose output |
| `-f`, `--force` | Force module loading |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Load a module:

```bash
modprobe snd_hda_intel
```

2. Load a module with parameters:

```bash
modprobe mymodule param1=value1
```

3. Remove a module:

```bash
modprobe -r mymodule
```

4. Show what would be done (dry run):

```bash
modprobe -n mymodule
```

5. Show current configuration:

```bash
modprobe -c
```

**FILES**

- `/etc/modprobe.conf` - Main configuration file
- `/etc/modprobe.d/` - Configuration directory
- `/lib/modules/$(uname -r)/modules.dep` - Module dependency database

**NOTES**

- Root privileges are required to load or remove modules.
- modprobe is preferred over insmod for its dependency handling.
- Configuration files can specify aliases, options, and install commands.

**SEE ALSO**

insmod(8), rmmod(8), lsmod(8), modinfo(8), depmod(8), kmod(8)

**PACKAGE**

kmod
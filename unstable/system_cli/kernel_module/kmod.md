# kmod

**NAME**

kmod - Linux kernel module management tool

**SYNOPSIS**

```bash
kmod [OPTIONS] [COMMAND] [COMMAND_OPTIONS]
```

**DESCRIPTION**

`kmod` is a multi-purpose tool for managing Linux kernel modules. Through various subcommands, it performs operations such as inserting, removing, listing, and managing dependencies of kernel modules.

kmod is a modern implementation that replaces the older module-init-tools, providing various module management functions through a single binary. Commands like lsmod, insmod, rmmod, modprobe, modinfo, and depmod are implemented as symbolic links to kmod.

**COMMANDS**

| Command | Description |
|---------|-------------|
| `help` | Display help message. |
| `list` | List currently loaded kernel modules (same as lsmod). |
| `static-nodes` | Display static node information. |

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Display help message. |
| `-V`, `--version` | Display version information. |
| `-q`, `--quiet` | Run in quiet mode. |
| `-v`, `--verbose` | Run in verbose mode. |

**BUILTIN COMMANDS**

kmod provides symbolic links for the following commands:

- **lsmod** - List loaded modules
- **insmod** - Insert module
- **rmmod** - Remove module
- **modprobe** - Probe module (automatic dependency resolution)
- **modinfo** - Show module information
- **depmod** - Generate module dependencies

**EXAMPLES**

1. Display help:

```bash
kmod --help
```

2. Check version information:

```bash
kmod --version
```

3. List loaded modules:

```bash
kmod list
# or
lsmod
```

4. Display static nodes information:

```bash
kmod static-nodes
```

**FILES**

- `/proc/modules` - Currently loaded kernel module information
- `/lib/modules/$(uname -r)/` - Kernel module directory
- `/lib/modules/$(uname -r)/modules.dep` - Module dependency information

**SEE ALSO**

lsmod(8), insmod(8), rmmod(8), modprobe(8), modinfo(8), depmod(8)

**PACKAGE**

kmod
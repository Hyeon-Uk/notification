# depmod

**NAME**

depmod - Generate modules.dep and map files

**SYNOPSIS**

```bash
depmod [OPTIONS]
depmod [OPTIONS] KERNEL_VERSION
```

**DESCRIPTION**

`depmod` creates a list of module dependencies by analyzing the symbols exported by each module and comparing them to the symbols needed by other modules. This generates the `modules.dep` file used by modprobe.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-a`, `--all` | Process all modules for all kernels |
| `-A`, `--quick` | Only process modules newer than modules.dep |
| `-b DIR`, `--basedir=DIR` | Use DIR as module base directory |
| `-e`, `--errsyms` | Report symbols that are not supplied |
| `-E SYMREGEX`, `--symbol-prefix=SYMREGEX` | Ignore symbols matching regex |
| `-F FILE`, `--filesyms=FILE` | Use symbol info from FILE (System.map) |
| `-n`, `--dry-run` | Don't write files, just print |
| `-v`, `--verbose` | Verbose output |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Generate dependencies for current kernel:

```bash
depmod
```

2. Generate dependencies for a specific kernel version:

```bash
depmod 5.4.0-generic
```

3. Dry run to see what would be generated:

```bash
depmod -n
```

4. Process all kernels:

```bash
depmod -a
```

5. Only update if modules are newer:

```bash
depmod -A
```

**FILES GENERATED**

| File | Description |
|------|-------------|
| `modules.dep` | Module dependency list |
| `modules.dep.bin` | Binary version of modules.dep |
| `modules.alias` | Module alias list |
| `modules.alias.bin` | Binary version of modules.alias |
| `modules.symbols` | Symbol aliases |
| `modules.symbols.bin` | Binary version of modules.symbols |
| `modules.builtin.alias.bin` | Built-in module aliases |
| `modules.devname` | Device names for module loading |

**FILES LOCATION**

Files are typically generated in:
- `/lib/modules/$(uname -r)/`

**NOTES**

- Root privileges are typically required to run depmod.
- Usually run automatically after installing new kernel modules.
- The `-A` option is useful for quick updates after installing a new module.

**SEE ALSO**

insmod(8), rmmod(8), modprobe(8), lsmod(8), modinfo(8), kmod(8)

**PACKAGE**

kmod
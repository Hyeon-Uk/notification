# rmmod

**NAME**

rmmod - Remove a loadable kernel module

**SYNOPSIS**

```bash
rmmod [OPTIONS] MODULE...
rmmod [OPTIONS] -a
```

**DESCRIPTION**

`rmmod` removes a loadable kernel module from the running kernel. By default, it will fail if the module is in use or has other modules depending on it.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-a`, `--all` | Remove all unused autoclean modules |
| `-f`, `--force` | Force removal (may cause system instability) |
| `-s`, `--syslog` | Send errors to syslog instead of stderr |
| `-v`, `--verbose` | Enable verbose output |
| `-w`, `--wait` | Wait until the module is no longer in use before removing |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Remove a kernel module:

```bash
rmmod mymodule
```

2. Force remove a module:

```bash
rmmod -f mymodule
```

3. Remove a module when it becomes unused:

```bash
rmmod -w mymodule
```

4. Remove all unused modules:

```bash
rmmod -a
```

**EXIT STATUS**

| Code | Description |
|------|-------------|
| 0 | Successfully removed the module |
| 1 | Module is in use or has dependent modules |
| 2 | Module not found or invalid |

**NOTES**

- Root privileges are required to remove kernel modules.
- Removing modules that are in use can cause system instability.
- Use lsmod to check if a module is in use before removing.

**SEE ALSO**

insmod(8), modprobe(8), lsmod(8), modinfo(8), kmod(8)

**PACKAGE**

kmod
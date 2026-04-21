# insmod

**NAME**

insmod - Install a loadable kernel module

**SYNOPSIS**

```bash
insmod [OPTIONS] FILE [ARGS...]
```

**DESCRIPTION**

`insmod` installs a loadable kernel module from the specified FILE into the running kernel. Unlike modprobe, insmod does not automatically resolve module dependencies.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-f`, `--force` | Force loading of module (ignore version and symbol checks) |
| `-s`, `--syslog` | Send errors to syslog instead of stderr |
| `-v`, `--verbose` | Enable verbose output |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Load a kernel module:

```bash
insmod /lib/modules/$(uname -r)/kernel/drivers/usb/usbcore.ko
```

2. Load a module with parameters:

```bash
insmod mymodule.ko param1=value1 param2=value2
```

3. Force load a module:

```bash
insmod -f mymodule.ko
```

**NOTES**

- insmod requires the full path to the module file (.ko).
- For modules with dependencies, use modprobe instead.
- Root privileges are required to load kernel modules.

**ERRORS**

| Error | Description |
|-------|-------------|
| Module already loaded | The module is already present in the kernel |
| Unknown symbol | Required symbols are not available in the kernel |
| Invalid module format | The module is not compatible with the running kernel |

**SEE ALSO**

rmmod(8), modprobe(8), lsmod(8), modinfo(8), kmod(8)

**PACKAGE**

kmod
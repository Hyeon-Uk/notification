# lsmod

**NAME**

lsmod - Show the status of modules in the Linux Kernel

**SYNOPSIS**

```bash
lsmod
```

**DESCRIPTION**

`lsmod` displays information about currently loaded kernel modules. It reads the `/proc/modules` file and presents the contents in a human-readable format.

**OUTPUT FORMAT**

The output consists of the following columns:

| Column | Description |
|--------|-------------|
| Module | The name of the module |
| Size | The size of the module in bytes |
| Used by | The count of references and the modules that use this module |

**EXAMPLES**

1. List all loaded kernel modules:

```bash
lsmod
```

2. Search for a specific module:

```bash
lsmod | grep snd
```

3. Count the number of loaded modules:

```bash
lsmod | wc -l
```

**FILES**

- `/proc/modules` - Information about loaded kernel modules

**SEE ALSO**

insmod(8), rmmod(8), modprobe(8), modinfo(8), kmod(8)

**PACKAGE**

kmod
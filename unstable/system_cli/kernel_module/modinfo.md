# modinfo

**NAME**

modinfo - Show information about a Linux Kernel module

**SYNOPSIS**

```bash
modinfo [OPTIONS] MODULE...
```

**DESCRIPTION**

`modinfo` displays information about a kernel module, including its filename, author, license, parameters, and other attributes from the module's metadata.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-a`, `--author` | Display only the module author |
| `-d`, `--description` | Display only the module description |
| `-l`, `--license` | Display only the module license |
| `-n`, `--filename` | Display only the module filename |
| `-p`, `--parameters` | Display only the module parameters |
| `-F FIELD`, `--field=FIELD` | Display only the specified field |
| `-0`, `--null` | Use null character as field separator |
| `-k KERNEL` | Use modules from KERNEL version |
| `-b DIR` | Use DIR as module base directory |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Show all information about a module:

```bash
modinfo snd_hda_intel
```

2. Show only the filename:

```bash
modinfo -n snd_hda_intel
```

3. Show module parameters:

```bash
modinfo -p snd_hda_intel
```

4. Show module author:

```bash
modinfo -a snd_hda_intel
```

**OUTPUT FIELDS**

| Field | Description |
|-------|-------------|
| filename | Path to the module file |
| license | Module license type |
| author | Module author |
| description | Module description |
| alias | Module aliases |
| depends | Module dependencies |
| parm | Module parameters with descriptions |

**SEE ALSO**

insmod(8), rmmod(8), modprobe(8), lsmod(8), kmod(8)

**PACKAGE**

kmod
# sysctl Specification

## 1. Overview
- **CLI Path:** `/usr/sbin/sysctl`
- **Execution Model:** Standard Utility
- **Description:** `sysctl` is a standard system utility for configuring kernel parameters at runtime. It reads and modifies kernel parameters from `/proc/sys/` directory. Parameters can be viewed, set, or loaded from configuration files.

## 2. Command Syntax
```bash
/usr/sbin/sysctl [OPTIONS] [VARIABLE[=VALUE]]
/usr/sbin/sysctl [OPTIONS] -p [FILE]
```

## 3. Global Options
| Option | Description |
|--------|-------------|
| `-a, --all` | display all values currently available |
| `-A, --deprecated` | alias of -a (deprecated) |
| `-b, --binary` | print value without new line |
| `--deprecated` | use deprecated proc file names |
| `-e, --ignore` | ignore unknown variable errors |
| `-f, --load[=FILE]` | read values from file or /etc/sysctl.conf |
| `-N, --names` | print only variable names |
| `-n, --values` | print only variable values |
| `-p, --load[=FILE]` | alias of -f |
| `-r, --pattern=EXPR` | only apply settings matching EXPR |
| `-q, --quiet` | do not echo variable set |
| `-S, --system` | read from all system directories |
| `-w, --write` | enable writing a value to variable |
| `-x, --deprecated[=yes\|no]` | strip deprecated proc file names |
| `--help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.
- Reads/writes from `/proc/sys/` directory.
- Configuration files in `/etc/sysctl.conf` and `/etc/sysctl.d/`.

## 6. Output Format
- **Format:** Text (Stdout)
- **Structure Details:** 
  - Variable name followed by value
  - Format: `variable.name = value`

**Example Output:**
```
kernel.hostname = tizen-device
kernel.osrelease = 5.15.0-tizen
kernel.version = #1 SMP PREEMPT Fri Jan 1 00:00:00 UTC 2026
net.ipv4.ip_forward = 0
vm.swappiness = 60
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: Display all kernel parameters
**Context:** List all available kernel parameters and their values.
```bash
$ sysctl -a
kernel.hostname = tizen-device
kernel.osrelease = 5.15.0-tizen
...
```

### Example 2: Display specific kernel parameter
**Context:** Check a specific kernel parameter value.
```bash
$ sysctl kernel.hostname
kernel.hostname = tizen-device
```

### Example 3: Set a kernel parameter temporarily
**Context:** Modify a kernel parameter at runtime.
```bash
$ sysctl -w vm.swappiness=50
vm.swappiness = 50
```

### Example 4: Load parameters from configuration file
**Context:** Apply system configuration from file.
```bash
$ sysctl -p /etc/sysctl.conf
vm.swappiness = 50
net.ipv4.ip_forward = 1
```

### Example 5: Display only values
**Context:** Get parameter value without name.
```bash
$ sysctl -n kernel.hostname
tizen-device
```

### Example 6: Apply system-wide configuration
**Context:** Load all sysctl configurations from system directories.
```bash
$ sysctl --system
* Applying /etc/sysctl.d/10-tizen.conf ...
vm.swappiness = 50
```

## 9. Package Information
- **Package:** procps-ng (or kmod on some systems)
- **Source:** https://gitlab.com/procps-ng/procps
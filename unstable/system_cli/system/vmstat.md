# vmstat Specification

## 1. Overview
- **CLI Path:** `/usr/bin/vmstat`
- **Execution Model:** Standard Utility
- **Description:** `vmstat` is a standard system utility included from the upstream `procps-ng` package. It reports virtual memory statistics, showing information about processes, memory, paging, block IO, traps, disks, and CPU activity.

## 2. Command Syntax
```bash
/usr/bin/vmstat [OPTIONS] [delay [count]]
```

## 3. Global Options
| Option | Description |
|--------|-------------|
| `-a, --active` | display active and inactive memory |
| `-f, --forks` | display number of forks since boot |
| `-m, --slabs` | display slabinfo |
| `-n, --one-header` | do not redisplay header |
| `-s, --stats` | display event counter and memory statistics |
| `-d, --disk` | display disk statistics |
| `-D, --disk-sum` | display disk summary statistics |
| `-p, --partition <device>` | display partition statistics |
| `-S, --unit <char>` | define display unit (k, K, m, M) |
| `-t, --timestamp` | show timestamp with output |
| `-w, --wide` | wide output mode |
| `-h, --help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.
- Reads system information from `/proc/meminfo`, `/proc/stat`, `/proc/vmstat`.

## 6. Output Format
- **Format:** Text (Stdout)
- **Structure Details:** 
  - **procs:** r (running), b (blocked)
  - **memory:** swpd (swap used), free, buff, cache
  - **swap:** si (swap in), so (swap out)
  - **io:** bi (block in), bo (block out)
  - **system:** in (interrupts), cs (context switches)
  - **cpu:** us (user), sy (system), id (idle), wa (wait), st (stolen)

**Example Output:**
```
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 1  0      0 123456  12345 234567    0    0    10    20  100  200  5 10 85  0  0
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: Display virtual memory statistics
**Context:** Check current memory and CPU usage.
```bash
$ vmstat
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 1  0      0 123456  12345 234567    0    0    10    20  100  200  5 10 85  0  0
```

### Example 2: Display statistics every 2 seconds
**Context:** Monitor system performance continuously.
```bash
$ vmstat 2
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 1  0      0 123456  12345 234567    0    0    10    20  100  200  5 10 85  0  0
 0  0      0 123456  12345 234567    0    0    15    25  110  210  4  9 87  0  0
```

### Example 3: Display active/inactive memory
**Context:** Check memory allocation details.
```bash
$ vmstat -a
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free  inact active   si   so    bi    bo   in   cs us sy id wa st
 1  0      0 123456 123456 234567    0    0    10    20  100  200  5 10 85  0  0
```

### Example 4: Display memory statistics summary
**Context:** View detailed memory event counters.
```bash
$ vmstat -s
     12345678 K total memory
      1234567 K used memory
       234567 K active memory
       123456 K inactive memory
      4567890 K free memory
       123456 K buffer memory
       234567 K swap cache
      1234567 K total swap
            0 K used swap
      1234567 K free swap
```

## 9. Package Information
- **Package:** procps-ng
- **Source:** https://gitlab.com/procps-ng/procps
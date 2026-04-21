# slabtop Specification

## 1. Overview
- **CLI Path:** `/usr/bin/slabtop`
- **Execution Model:** Standard Utility
- **Description:** `slabtop` is a standard system utility included from the upstream `procps-ng` package. It displays detailed kernel slab cache information in real-time, showing how memory is being used by the kernel's slab allocator.

## 2. Command Syntax
```bash
/usr/bin/slabtop [OPTIONS]
```

## 3. Global Options
| Option | Description |
|--------|-------------|
| `-o, --order` | sort by active objects (descending) |
| `-s, --sort=SORT` | specify sort criteria: a (active), b (objsize), c (cache_size), l (limit), n (num_obj), p (pages), s (objsize), v (active_objs), z (zero) |
| `-n, --once` | run once and exit (non-interactive) |
| `-d, --delay=SECONDS` | update interval in seconds (default: 3) |
| `-h, --help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.
- Reads system information from `/proc/slabinfo`.

## 6. Output Format
- **Format:** Text (Stdout) - Interactive display
- **Structure Details:** 
  - **Header:** Active / Total objects, active / total slabs, active / total caches, cache size summary
  - **Columns:**
    - **OBJS:** Total number of objects allocated
    - **ACTIVE:** Number of objects currently active
    - **USE:** Percentage of active objects
    - **OBJ SIZE:** Size of each object in bytes
    - **SLABS:** Number of slab pages allocated
    - **OBJ/SLAB:** Objects per slab
    - **CACHE SIZE:** Total cache size
    - **NAME:** Cache name

**Example Output:**
```
 Active / Total Objects (% used)    : 123456 / 234567 (52.6%)
 Active / Total Slabs (% used)      : 12345 / 23456 (52.6%)
 Active / Total Caches (% used)     : 123 / 456 (27.0%)
 Active / Total Size (% used)       : 1234.56M / 2345.67M (52.6%)
 Minimum / Average / Maximum Object : 0.01K / 0.19K / 8.00K

  OBJS ACTIVE  USE OBJ SIZE  SLABS OBJ/SLAB CACHE SIZE NAME
 12345   10000  81%    0.19K   1234       10      4936K dentry
  5678    5000  88%    0.12K    567       10      2268K inode_cache
  2345    2000  85%    0.25K    234       10       936K buffer_head
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: Display slab cache information
**Context:** Monitor kernel slab cache usage in real-time.
```bash
$ slabtop
 Active / Total Objects (% used)    : 123456 / 234567 (52.6%)
 Active / Total Slabs (% used)      : 12345 / 23456 (52.6%)
 ...
```

### Example 2: Display once and exit
**Context:** Get a snapshot of slab cache information.
```bash
$ slabtop -n
 Active / Total Objects (% used)    : 123456 / 234567 (52.6%)
 ...
```

### Example 3: Sort by cache size
**Context:** Find largest kernel caches.
```bash
$ slabtop -s c
```

### Example 4: Sort by active objects
**Context:** Find most active caches.
```bash
$ slabtop -s a
```

## 9. Package Information
- **Package:** procps-ng
- **Source:** https://gitlab.com/procps-ng/procps
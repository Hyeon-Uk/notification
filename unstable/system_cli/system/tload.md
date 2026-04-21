# tload Specification

## 1. Overview
- **CLI Path:** `/usr/bin/tload`
- **Execution Model:** Standard Utility
- **Description:** `tload` is a standard system utility included from the upstream `procps-ng` package. It displays a graphical representation of the system load average in a terminal, showing the load as a rising and falling graph over time.

## 2. Command Syntax
```bash
/usr/bin/tload [OPTIONS] [TTY]
```

## 3. Global Options
| Option | Description |
|--------|-------------|
| `-s, --scale N` | vertical scale factor (number of characters per load unit) |
| `-d, --delay N` | delay between updates in seconds (default: 1) |
| `-h, --help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.
- Reads system information from `/proc/loadavg`.

## 6. Output Format
- **Format:** Text (Stdout) - Interactive display
- **Structure Details:** 
  - A graph made of asterisks (*) representing the load average
  - The graph scrolls from right to left
  - Higher load = more asterisks vertically
  - Load average numbers displayed at the bottom

**Example Output:**
```
                                                     
                                                     
                                                     
                                            *        
                                            *        
                                            *        
                                        *   *        
                                        *   *        
                                    *   *   *        
                                *   *   *   *        
                            *   *   *   *   *        
                        *   *   *   *   *   *        
                    *   *   *   *   *   *   *        
                *   *   *   *   *   *   *   *        
            *   *   *   *   *   *   *   *   *        
        *   *   *   *   *   *   *   *   *   *        
    *   *   *   *   *   *   *   *   *   *   *        
1.23, 1.45, 1.67
```

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: Display system load graphically
**Context:** Visual representation of system load.
```bash
$ tload
[Graph displays with asterisks representing load]
```

### Example 2: Set update delay to 5 seconds
**Context:** Slower update rate for monitoring.
```bash
$ tload -d 5
[Graph updates every 5 seconds]
```

### Example 3: Set vertical scale
**Context:** Adjust the sensitivity of the graph.
```bash
$ tload -s 10
[Graph with 10 characters per load unit]
```

### Example 4: Display on specific terminal
**Context:** Show load graph on a different terminal.
```bash
$ tload /dev/pts/1
[Graph displays on specified terminal]
```

## 9. Package Information
- **Package:** procps-ng
- **Source:** https://gitlab.com/procps-ng/procps
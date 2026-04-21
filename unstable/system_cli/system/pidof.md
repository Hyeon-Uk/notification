# pidof Specification

## 1. Overview
- **CLI Path:** `/usr/bin/pidof`
- **Execution Model:** Standard Utility
- **Description:** `pidof` is a standard system utility included essentially from the upstream `procps-ng` package. It offers basic command functionalities.

## 2. Command Syntax
```bash
/usr/bin/pidof [OPTIONS]
```

## 3. Global Options
| Option                        | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| `-s, --single-shot` | return one PID only |
| `-c, --check-root` | omit processes with different root |
| `-q,` | quiet mode, only set the exit code |
| `-w, --with-workers` | show kernel workers too |
| `-x` | also find shells running the named scripts |
| `-o, --omit-pid <PID,...>` | omit processes with PID |
| `-t, --lightweight` | list threads too |
| `-S, --separator SEP` | use SEP as separator put between PIDs |
| `-h, --help` | display this help and exit |
| `-V, --version` | output version information and exit |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Uses standard Linux kernel environment pathways.

## 6. Output Format
- **Format:** Text (Stdout)
- **Structure Details:** Outputs basic process or file status logs.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Command execution completed without standard errors. |
| `<nonzero>` | Error | Command experienced a logic error or syntax problem. |

## 8. Practical Examples

### Example 1: View standard help
**Context:** Check binary usage syntax.
```bash
$ pidof --help
```

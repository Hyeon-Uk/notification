# free Specification

## 1. Overview
- **CLI Path:** `/usr/bin/free`
- **Execution Model:** Standard Utility
- **Description:** `free` is a standard system utility included essentially from the upstream `procps-ng` package. It offers basic command functionalities.

## 2. Command Syntax
```bash
/usr/bin/free [OPTIONS]
```

## 3. Global Options
| Option                        | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| `-b, --bytes` | show output in bytes |
| `--kilo` | show output in kilobytes |
| `--mega` | show output in megabytes |
| `--giga` | show output in gigabytes |
| `--tera` | show output in terabytes |
| `--peta` | show output in petabytes |
| `-k, --kibi` | show output in kibibytes |
| `-m, --mebi` | show output in mebibytes |
| `-g, --gibi` | show output in gibibytes |
| `--tebi` | show output in tebibytes |
| `--pebi` | show output in pebibytes |
| `-h, --human` | show human-readable output |
| `--si` | use powers of 1000 not 1024 |
| `-l, --lohi` | show detailed low and high memory statistics |
| `-L, --line` | show output on a single line |
| `-t, --total` | show total for RAM + swap |
| `-v, --committed` | show committed memory and commit limit |
| `-s N, --seconds N` | repeat printing every N seconds |
| `-c N, --count N` | repeat printing N times, then exit |
| `-w, --wide` | wide output |
| `--help` | display this help and exit |
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
$ free --help
```

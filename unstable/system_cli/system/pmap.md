# pmap Specification

## 1. Overview
- **CLI Path:** `/usr/bin/pmap`
- **Execution Model:** Standard Utility
- **Description:** `pmap` is a standard system utility included essentially from the upstream `procps-ng` package. It offers basic command functionalities.

## 2. Command Syntax
```bash
/usr/bin/pmap [OPTIONS]
```

## 3. Global Options
| Option                        | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| `-x, --extended` | show details |
| `-X` | show even more details |
| `-XX` | show everything the kernel provides |
| `-c, --read-rc` | read the default rc |
| `-C, --read-rc-from=<file>` | read the rc from file |
| `-n, --create-rc` | create new default rc |
| `-N, --create-rc-to=<file>` | create new rc to file |
| `-d, --device` | show the device format |
| `-q, --quiet` | do not display header and footer |
| `-p, --show-path` | show path in the mapping |
| `-A, --range=<low>[,<high>]` | limit results to the given range |
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
$ pmap --help
```

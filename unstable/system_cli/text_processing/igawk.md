# igawk Specification

## 1. Overview
- **CLI Path:** `/usr/bin/igawk`
- **Execution Model:** Filter / Script Interpreter
- **Description:** `igawk` is a shell script wrapper for the GNU `awk` utility that seamlessly provides the ability to include (`@include`) external library files within AWK scripts. It preprocesses the input awk script, resolves `@include "filename"` statements by dynamically injecting the file contents, and then passes the expanded program to `gawk`.

## 2. Command Syntax
```bash
igawk [options] 'program text' [file ...]
igawk [options] -f program-file [file ...]
```

## 3. Global Options
| Option        | Description                                                                                                                                                                                                                                                                     |
|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `--`          | Signals the end of `igawk` specific options; the rest of the arguments are passed directly to `gawk`. |
| `-W <arg>`    | Pass a `gawk`-specific warning or POSIX conformance argument. |
| `(pass-through)` | All standard `gawk` options (e.g., `-F`, `-v`, `-f`) are fully supported and passed natively. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Reads the `AWKPATH` environment variable to search for dynamically included `.awk` library files.

## 6. Output Format
- **Format:** Native `gawk` text output.
- **Structure Details:** The wrapper itself produces no visible output unless preprocessing fails (e.g., an include file cannot be read). Everything else is determined by the underlying AWK script.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Execution of the underlying AWK script succeeded. |
| `<nonzero>` | Error | Failed to parse includes, or the AWK script crashed/exited with a non-zero status. |

## 8. Practical Examples

### Example 1: Run an AWK script with include directives
**Context:** The `report.awk` file contains an `@include "common.awk"` line. Run it via igawk to resolve the library correctly.
```bash
$ igawk -f report.awk input.txt
```

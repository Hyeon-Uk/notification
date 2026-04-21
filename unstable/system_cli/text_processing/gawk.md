# gawk Specification

## 1. Overview
- **CLI Path:** `/usr/bin/gawk`
- **Execution Model:** Filter / Script Interpreter
- **Description:** `gawk` is the GNU implementation of the AWK programming language. It is a powerful structural text processing framework used to filter output, handle regexes, and manipulate streams of text data in Tizen shell scripts. 

## 2. Command Syntax
```bash
gawk [POSIX or GNU style options] -f progfile [--] file ...
gawk [POSIX or GNU style options] [--] 'program' file ...
```

## 3. Global Options
| Option        | Description                                                                                                                                                                                                                                                                     |
|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `-f file`     | Read the AWK program source from the given file instead of passing it inline. |
| `-F fs`       | Defines the input field separator character or regex (sets the built-in `FS` variable). |
| `-v var=val`  | Assigns a value to a program variable prior to script compilation. |
| `--lint`      | Enable warnings about constructs that are dubious or non-portable to other awk implementations. |
| `--posix`     | Turn on strict POSIX compatibility mode. Disables GNU extensions dynamically. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Relies on `AWKPATH` for locating `-f` programs. Uses locale variables (`LC_ALL`, `LANG`) for proper string encodings and comparisons.

## 6. Output Format
- **Format:** Processed Text (Stdout/Stderr)
- **Structure Details:** Outputs fully depend on the executed `print` or `printf` directives within the script.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Script executed without encountering fatal I/O or syntax errors. |
| `<nonzero>` | Error | Halting due to a syntax error, illegal variable assignment, or forced script `exit N` instructions. |

## 8. Practical Examples

### Example 1: Count lines matching a pattern
**Context:** Check the system messages log and count how many times the word "error" appears as the root user.
```bash
$ gawk '/error/ { count++ } END { print count }' /var/log/messages
```

# awk Specification

## 1. Overview
- **CLI Path:** `/usr/bin/awk`
- **Execution Model:** Filter / Script Interpreter
- **Description:** `awk` is the standard POSIX pattern scanning and processing language utility. On Tizen, it is typically a symlink to GNU `gawk`. It operates on streams or text files line by line, evaluating patterns and executing corresponding actions to extract, format, or calculate data.

## 2. Command Syntax
```bash
awk [POSIX or GNU style options] -f progfile [--] file ...
awk [POSIX or GNU style options] [--] 'program' file ...
```

## 3. Global Options
| Option        | Description                                                                                                                                                                                                                                                                     |
|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `-f file`     | Read the AWK program source from the specified file instead of the command-line string. |
| `-v var=val`  | Assign the value `val` to the variable `var` before execution of the program begins. |
| `-F fs`       | Set the input field separator string (`FS`) to the regular expression `fs`. |
| `--`          | Signal the end of options. Subsequent arguments are treated as input files or variables. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Supports standard `AWKPATH` for scripts and `AWKLIBPATH` for shared libraries. Heavily relies on `POSIXLY_CORRECT` for strict standard enforcement.

## 6. Output Format
- **Format:** Formatted text (`stdout`).
- **Structure Details:** Arbitrary text processing output depending on the execution script. Includes standard data column formats or JSON outputs if using specific libraries.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Normal execution and parsing complete. |
| `1` (or `2`) | Error | Syntax errors in the script or failure processing specific I/O files. |

## 8. Practical Examples

### Example 1: Extract specific columns
**Context:** Extract the process name and memory from the system ps output.
```bash
$ ps aux | awk '{print $1, $4, $11}'
```

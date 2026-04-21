# sed Specification

## 1. Overview
- **CLI Path:** `/usr/bin/sed`
- **Execution Model:** One-shot / Streaming
- **Description:** Standard stream editor for filtering and transforming text. It applies specified script commands to an input stream or files and outputs the transformed text.

## 2. Command Syntax
```bash
/usr/bin/sed [OPTION]... {script-only-if-no-other-script} [input-file]...
```

## 3. Global Options
| Flag | Short | Type | Default | Description |
|------|-------|------|---------|-------------|
| `--quiet`, `--silent` | `-n` | Boolean | `false` | Suppress automatic printing of pattern space |
| `--expression` | `-e` | String | None | Add the script to the commands to be executed |
| `--file` | `-f` | String | None | Add the contents of script-file to the commands to be executed |
| `--follow-symlinks` | | Boolean | `false` | Follow symlinks when processing in place |
| `--in-place` | `-i` | String | None | Edit files in place (makes backup if extension supplied) |
| `--line-length` | `-l` | Integer | None | Specify the desired line-wrap length for the 'l' command |
| `--posix` | | Boolean | `false` | Disable all GNU extensions |
| `--regexp-extended` | `-r` | Boolean | `false` | Use extended regular expressions in the script |
| `--separate` | `-s` | Boolean | `false` | Consider files as separate rather than as a single continuous long stream |
| `--unbuffered` | `-u` | Boolean | `false` | Load minimal amounts of data from the input files and flush the output buffers more often |
| `--help` | | Boolean | `false` | Display help and exit |
| `--version` | | Boolean | `false` | Output version information and exit |

## 4. Commands (if applicable)
Single-purpose command, no strict subcommands.

## 5. Environment Variables & Configuration
- None explicitly required. Uses standard stdin/stdout.

## 6. Output Format
- **Format:** Text
- **Structure Details:** Outputs the transformed text stream based on the provided sed script rules.

## 7. Exit Status Codes
| Code | Name | Description |
|------|------|-------------|
| `0` | Success | The operation completed successfully. |
| `<nonzero>` | General Error | Execution failed. |

## 8. Practical Examples
### Example 1: Basic String Replacement
**Context:** Replace all occurances of 'foo' with 'bar' in a stream.
```bash
$ echo "hello foo" | sed -e 's/foo/bar/g'
```
**Output (`stdout`):**
```text
hello bar
```

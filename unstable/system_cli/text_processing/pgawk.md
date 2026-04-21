# pgawk Specification

## 1. Overview
- **CLI Path:** `/usr/bin/pgawk`
- **Execution Model:** Filter / Profiler wrapper
- **Description:** `pgawk` is the profiling version of GNU `awk`. It is designed to act identically to standard `gawk` but produces an execution profile of the awk script being run, allowing developers to see execution counts of rules and statements. Useful for debugging complex text-processing pipelines in Tizen.

## 2. Command Syntax
```bash
pgawk [POSIX or GNU style options] -f progfile [--] file ...
```

## 3. Global Options
| Option        | Description                                                                                                                                                                                                                                                                     |
|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `--profile[=file]` | Produce the profiling report in `awkprof.out` or the explicitly designated file name. |
| `-f file`     | Load the script logic from a separate file to be parsed and profiled. |
| `-F fs`       | Override the field grouping character regex. |
| `-v var=val`  | Instantiate internal pre-execution variables dynamically. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Maintains identical configuration logic to `gawk`, using `AWKPATH`.

## 6. Output Format
- **Format:** Dual Text Format (Stdout standard output, side-effect profile text file).
- **Structure Details:** The execution output remains normally undisturbed. The profiling file (`awkprof.out`) contains a pretty-printed version of the awk script appended with execution counts alongside each block and standard rule.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Evaluated successfully and successfully dumped the execution profile data. |
| `<nonzero>` | Error | Syntax failures inside the target awk script. |

## 8. Practical Examples

### Example 1: Profile a complex script
**Context:** Track how many times a given regex branch invokes processing for large logging text bases.
```bash
$ pgawk --profile=report.prof -f analyzer.awk system.log
```

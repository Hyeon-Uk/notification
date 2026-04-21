# pgawk-3.1.5 Specification

## 1. Overview
- **CLI Path:** `/usr/bin/pgawk-3.1.5`
- **Execution Model:** Filter / Profiler wrapper
- **Description:** `pgawk-3.1.5` is the specific versioned binary of the profiling GNU awk interpreter. It maintains strict feature compatibility with the v3.1.5 internal release, providing stable execution statistics profiling without breaking long-standing internal syntax extensions used in Tizen scripts.

## 2. Command Syntax
```bash
pgawk-3.1.5 [POSIX or GNU style options] -f progfile [--] file ...
```

## 3. Global Options
| Option        | Description                                                                                                                                                                                                                                                                     |
|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `--profile[=file]` | Enable the generation of the performance profile report file upon completion. Defaults to `awkprof.out`. |
| `-f file`     | Target the specific awk text processor script to profile. |
| `-F fs`       | Target custom field separator definitions. |
| `-v var=val`  | Push pre-execution assignment constants into the variables memory tree. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Supports legacy fallback configurations over `AWKPATH`.

## 6. Output Format
- **Format:** Dual Text Format (Stdout normal, profiling side-effect file text format)
- **Structure Details:** Modifies no internal script streams, producing stdout cleanly while exporting exact loop and conditional branch counts.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Success in processing the input files and generating the output counts. |
| `<nonzero>` | Error | Parsing blocks failed structurally. |

## 8. Practical Examples

### Example 1: Stable profiling of legacy scripts
**Context:** Generate a simple profile report to evaluate heavy string manipulation loops.
```bash
$ echo "test" | pgawk-3.1.5 --profile=legacy.prof '{ for(i=0; i<3; i++) print $1 }'
```

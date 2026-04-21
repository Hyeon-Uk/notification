# hal-compatibility-checker Specification

## 1. Overview
- **CLI Path:** `/usr/bin/hal-compatibility-checker`
- **Execution Model:** **One-shot**
- **Description:** `hal-compatibility-checker` validates the compatibility between the platform HAL API version and the installed HAL backend modules. It detects version mismatches that could cause runtime failures, and caches results for efficiency.

## 2. Command Syntax
```bash
hal-compatibility-checker [options]
```

## 3. Global Options

| Flag | Type | Description |
|------|------|-------------|
| `-h, --help` | Boolean | Show help message. |
| `--skip-if-result-exist[=FILE]` | String | Skip check if a result file already exists. If FILE is given, uses that specific file. |
| `--reset` | Boolean | Remove the existing result file and re-check. |

## 4. Commands (if applicable)
Single-purpose command. Without options, performs the compatibility check and reports results.

## 5. Environment Variables & Configuration
- HAL backend modules are installed under `/hal/lib/`.
- Compatibility results are cached for subsequent runs.

## 6. Output Format
- **Format:** Text (stdout)
- **Structure Details:** Reports HAL compatibility status for each module.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | All HAL modules are compatible. |
| `1` | Error | Compatibility mismatch detected. |

## 8. Practical Examples

### Example 1: Run compatibility check
```bash
$ hal-compatibility-checker
```

### Example 2: Skip if already checked
```bash
$ hal-compatibility-checker --skip-if-result-exist
```

### Example 3: Force re-check
```bash
$ hal-compatibility-checker --reset
```

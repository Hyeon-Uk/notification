# parse-dynparts Specification

## 1. Overview
- **CLI Path:** `/usr/sbin/parse-dynparts`
- **Execution Model:** One-shot
- **Description:** A Tizen-specific upgrade scripting utility. Computes dynamic partition offsets natively during firmware flashing routines (FOTA/Upgrade).

## 2. Command Syntax
```bash
/usr/sbin/parse-dynparts [FILE]
```

## 3. Global Options
Accepts specific partition manifest tables.

## 4. Commands (if applicable)
Parses strictly defined structured layouts.

## 5. Environment Variables & Configuration
- Executes heavily inside the `upgrade` platform repository paths.

## 6. Output Format
- **Format:** Text
- **Structure Details:** Prints calculated partition offsets for the flashing pipeline.

## 7. Exit Status Codes
| Code | Name | Description |
|------|------|-------------|
| `0` | Success | The operation completed successfully. |
| `<nonzero>` | General Error | Execution failed. |

## 8. Practical Examples
### Example 1: Extract partition rules
**Context:** Run natively during system updates.
```bash
$ parse-dynparts parts.xml
```

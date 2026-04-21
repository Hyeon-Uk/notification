# reboot-haltest Specification

## 1. Overview
- **CLI Path:** `/usr/bin/reboot-haltest`
- **Execution Model:** Shell Script Trigger
- **Description:** `reboot-haltest` invokes `systemd` to route the next emulator startup specifically into `haltest.target` instead of `graphical.target`.

## 2. Command Syntax
```bash
/usr/bin/reboot-haltest
```

## 3. Global Options
| Option                        | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| *(None explicitly defined)*   | Script takes zero arguments. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Modifies `/etc/systemd/system/default.target`.

## 6. Output Format
- **Format:** Shell Messages
- **Structure Details:** Outputs target transitions and triggers reboot.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | System reboot triggered. |
| `<nonzero>` | Error | Permission denied. |

## 8. Practical Examples

### Example 1: Reboot into hardware diagnostics
**Context:** Quickly bypass the UI.
```bash
$ sdb shell su -c "/usr/bin/reboot-haltest"
```

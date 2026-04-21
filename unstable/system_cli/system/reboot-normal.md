# reboot-normal Specification

## 1. Overview
- **CLI Path:** `/usr/bin/reboot-normal`
- **Execution Model:** Shell Script Trigger
- **Description:** `reboot-normal` invokes `systemd` to route the next emulator startup into `graphical.target` (normal boot mode). This is used to restore normal boot behavior after using `reboot-haltest`.

## 2. Command Syntax
```bash
/usr/bin/reboot-normal
```

## 3. Global Options
| Option                        | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| *(None explicitly defined)*   | Script takes zero arguments. |

## 4. Commands (if applicable)
*(No discrete subcommands)*

## 5. Environment Variables & Configuration
- Modifies `/etc/systemd/system/default.target` to point to `graphical.target`.

## 6. Output Format
- **Format:** Shell Messages
- **Structure Details:** Outputs target transitions and triggers reboot.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | System reboot triggered. |
| `<nonzero>` | Error | Permission denied. |

## 8. Practical Examples

### Example 1: Reboot into normal graphical mode
**Context:** Restore normal boot after haltest mode.
```bash
$ sdb shell su -c "/usr/bin/reboot-normal"
```

### Example 2: Switch from haltest to normal mode
**Context:** After completing hardware diagnostics, return to normal operation.
```bash
# First check current target
$ sdb shell systemctl get-default
haltest.target

# Reboot to normal mode
$ sdb shell su -c "/usr/bin/reboot-normal"
```

## 9. Related Tools
- `reboot-haltest` - Reboot into HAL test mode
- `systemctl` - Control the systemd system and service manager

## 10. Package Information
- **Package:** Tizen platform-specific
- **See Also:** `reboot-haltest`, `systemctl`
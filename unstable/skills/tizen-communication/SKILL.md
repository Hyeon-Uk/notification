---
name: tizen-communication
description: Manages telephony operations including calls, SMS, SIM card information, and call answering. Use when working with phone calls, SMS, telephony operations, or call management.
---

# Tizen Communication

Manages telephony operations including calls, SMS, and SIM card information.

## Quick reference

**Telephony**: Telephony operations → `tizen-telephony-cli`
**YACA**: Call answering → `tizen-yaca-cli`

## Telephony

Manage telephony operations:

```bash
# Get SIM state
tizen-telephony-cli sim-state

# Get SIM info
tizen-telephony-cli sim-info

# Get network info
tizen-telephony-cli network-info

# Get modem status
tizen-telephony-cli modem-status
```

See [tizen-telephony-cli.md](references/tizen-telephony-cli.md) for complete documentation.

## YACA (Yet Another Call Answer)

Manage call answering:

```bash
# Answer call
tizen-yaca-cli answer

# Reject call
tizen-yaca-cli reject

# Get call status
tizen-yaca-cli status
```

See [tizen-yaca-cli.md](references/tizen-yaca-cli.md) for complete documentation.

## Common use cases

- **Dialer applications**: Manage phone calls
- **SMS apps**: Handle SMS operations
- **Call management**: Control call answering and rejection

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
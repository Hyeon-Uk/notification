---
name: tizen-legacy
description: Handles legacy and specialized functionality including MMI operations and MTP device management. Use when working with MMI protocol, MTP devices, or specialized hardware communication.
---

# Tizen Legacy

Handles legacy and specialized functionality including MMI and MTP operations.

## Quick reference

**MMI**: MMI operations → `tizen-mmi-cli`
**MTP**: MTP operations → `tizen-mtp-cli`

## MMI (Man-Machine Interface)

Manage MMI protocol operations:

```bash
# Get MMI state
tizen-mmi-cli get-state

# Send MMI command
tizen-mmi-cli send --command "*#06#"

# Get MMI info
tizen-mmi-cli info
```

See [tizen-mmi-cli.md](references/tizen-mmi-cli.md) for complete documentation.

## MTP (Media Transfer Protocol)

Manage MTP device connections:

```bash
# List MTP devices
tizen-mtp-cli list-devices

# Get device info
tizen-mtp-cli device-info --device-index 0

# List storages
tizen-mtp-cli list-storages --device-index 0

# Get storage info
tizen-mtp-cli storage-info --storage-index 0 --device-index 0

# List objects
tizen-mtp-cli list-object-handles --storage-index 0 --parent 0
```

See [tizen-mtp-cli.md](references/tizen-mtp-cli.md) for complete documentation.

## Common use cases

- **Legacy integration**: MMI protocol operations
- **MTP device management**: Transfer files via MTP
- **Specialized hardware**: MTP device communication

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
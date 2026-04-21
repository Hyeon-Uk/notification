---
name: tizen-system-info
description: Queries Tizen system information, platform features, runtime states, and system settings. Use when the user asks about system capabilities, runtime information (CPU, memory), platform features, or system settings.
argument-hint: <FEATURE_KEY|SETTING_KEY>
user-invocable: true
allowed-tools: true
---

# Tizen System Info

Query system information, platform features, runtime states, and settings using these CLI tools:

- **`tizen-system-info-cli`** — Platform feature keys and capabilities
- **`tizen-runtime-info-cli`** — Runtime states (CPU, memory, processor)
- **`tizen-base-utils-cli`** — Common utility functions
- **`tizen-bundle-cli`** — Bundle data management
- **`tizen-capi-system-system-settings-cli`** — System settings
- **`tizen-tzsh-quickpanel-cli`** — Quickpanel control

## Quick Start

```bash
# Query platform features
tizen-system-info-cli get-bool http://tizen.org/feature/network.wifi
tizen-system-info-cli feature-set --group connectivity

# Query runtime information
tizen-runtime-info-cli status
tizen-runtime-info-cli memory
tizen-runtime-info-cli cpu
tizen-runtime-info-cli processor

# System settings
tizen-capi-system-system-settings-cli get http://tizen.org/setting/locale.language
tizen-capi-system-system-settings-cli list
```

## System Info vs Runtime Info

| Tool | Purpose | Use When |
|------|---------|----------|
| `tizen-system-info-cli` | Static platform features | Checking if feature is supported |
| `tizen-runtime-info-cli` | Dynamic runtime states | Querying CPU, memory, battery status |

**System Info** queries static platform capabilities (e.g., "Does this device support WiFi?").
**Runtime Info** queries dynamic runtime states (e.g., "How much memory is available now?").

## Common Use Cases

| User Request | Command |
|--------------|---------|
| "Does this device support WiFi?" | `tizen-system-info-cli get-bool http://tizen.org/feature/network.wifi` |
| "How much memory is available?" | `tizen-runtime-info-cli memory` |
| "What's the CPU usage?" | `tizen-runtime-info-cli cpu` |
| "Get system language setting" | `tizen-capi-system-system-settings-cli get http://tizen.org/setting/locale.language` |
| "List all connectivity features" | `tizen-system-info-cli feature-set --group connectivity` |
| "Check processor info" | `tizen-runtime-info-cli processor` |

## Workflows

### System Capability Check

```bash
# Check if feature is supported
tizen-system-info-cli get-bool http://tizen.org/feature/camera

# Get feature details
tizen-system-info-cli feature-set --group display
```

### Runtime Diagnostics

```bash
# Check system status
tizen-runtime-info-cli status

# Check memory
tizen-runtime-info-cli memory

# Check CPU usage
tizen-runtime-info-cli cpu
```

## References

- [tizen-system-info-cli.md](references/tizen-system-info-cli.md)
- [tizen-runtime-info-cli.md](references/tizen-runtime-info-cli.md)
- [tizen-base-utils-cli.md](references/tizen-base-utils-cli.md)
- [tizen-bundle-cli.md](references/tizen-bundle-cli.md)
- [tizen-capi-system-system-settings-cli.md](references/tizen-capi-system-system-settings-cli.md)
- [tizen-tzsh-quickpanel-cli.md](references/tizen-tzsh-quickpanel-cli.md)

## When to Use

- User asks about system capabilities or features
- User needs runtime information (CPU, memory, processor)
- User wants to check or change system settings
- User needs to query platform-level information

## When NOT to Use

- User wants to manage applications → use `tizen-application-management` skill
- User wants to access sensor data → use `tizen-hardware` skill
- User wants to manage network connections → use `tizen-network` skill
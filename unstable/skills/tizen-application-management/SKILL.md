---
name: tizen-application-management
description: Manages Tizen applications and packages including launch, terminate, list, install, and remove operations. Use when the user asks about running applications, wants to launch or terminate apps, or needs to install/remove packages.
argument-hint: <APPLICATION_ID|PACKAGE_NAME>
user-invocable: true
allowed-tools: true
---

# Tizen Application Management

Manage Tizen applications and packages using these CLI tools:

- **`tizen-app-manager-cli`** — Application lifecycle, context queries, termination
- **`tizen-app-control-cli`** — App launch and control operations
- **`tizen-app-event-cli`** — Application event monitoring
- **`tizen-package-manager-cli`** — Package installation, removal, and management

## Quick Start

```bash
# List running applications
tizen-app-manager-cli list-running

# List installed applications
tizen-app-manager-cli list-installed

# Launch application
tizen-app-control-cli launch --app-id <APP_ID>

# Terminate application
tizen-app-manager-cli terminate --app-id <APP_ID>

# Install package
tizen-package-manager-cli install <PACKAGE_FILE>

# Remove package
tizen-package-manager-cli remove <PACKAGE_ID>
```

## Common Use Cases

| User Request | Command |
|--------------|---------|
| "What apps are running?" | `tizen-app-manager-cli list-running` |
| "Launch Netflix" | `tizen-app-control-cli launch --app-id org.tizen.netflix` |
| "Stop YouTube" | `tizen-app-manager-cli terminate --app-id org.tizen.youtube` |
| "Install this package" | `tizen-package-manager-cli install <PACKAGE_FILE>` |
| "What's the current app?" | `tizen-app-manager-cli get-focused` |
| "Show me all apps" | `tizen-app-manager-cli list-installed` |

## Workflows

### Launch and Monitor Application

```bash
# Launch application
tizen-app-control-cli launch --app-id <APP_ID>

# Verify launch
tizen-app-manager-cli get-focused

# Monitor events (optional)
tizen-app-event-cli monitor --app-id <APP_ID>
```

### Package Installation

```bash
# Install package
tizen-package-manager-cli install <PACKAGE_FILE>

# Verify installation
tizen-package-manager-cli info <PACKAGE_ID>

# Launch to verify
tizen-app-control-cli launch --app-id <APP_ID>
```

## References

- [tizen-app-manager-cli.md](references/tizen-app-manager-cli.md)
- [tizen-app-control-cli.md](references/tizen-app-control-cli.md)
- [tizen-app-event-cli.md](references/tizen-app-event-cli.md)
- [tizen-package-manager-cli.md](references/tizen-package-manager-cli.md)

## When to Use

- User wants to launch, terminate, or manage applications
- User needs to install or remove packages
- User asks about running applications or app state

## When NOT to Use

- User wants to search or play content → use `tizen-content-data` skill
- User wants to manage network or Wi-Fi → use `tizen-network` skill
- User wants to handle audio/media → use `tizen-media-player` skill
- User wants to control UI or input methods → use `tizen-ui-input` skill
- User needs system runtime information → use `tizen-system-info` skill
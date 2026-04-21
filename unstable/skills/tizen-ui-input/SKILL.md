---
name: tizen-ui-input
description: Manages user interface input methods, notifications, haptic feedback, and autofill functionality. Use when working with virtual keyboards, input methods, notifications, or UI feedback.
---

# Tizen UI Input

Manages input methods, notifications, haptic feedback, and autofill functionality.

## Quick reference

**UI Autofill**: Autofill CAPI → `tizen-capi-ui-autofill-cli`
**UI Input Method**: Input method CAPI → `tizen-capi-ui-inputmethod-cli`
**UI Input Method Manager**: Input method manager → `tizen-capi-ui-inputmethod-manager-cli`
**LibFeedback**: Haptic feedback → `tizen-libfeedback-cli`
**Notification**: Notification management → `tizen-notification-cli`

## UI Autofill

Manage autofill functionality:

```bash
# Get autofill data
tizen-capi-ui-autofill-cli get --field username

# Set autofill data
tizen-capi-ui-autofill-cli set --field username --value user123

# Clear autofill data
tizen-capi-ui-autofill-cli clear --field username
```

See [tizen-capi-ui-autofill-cli.md](references/tizen-capi-ui-autofill-cli.md) for complete documentation.

## UI Input Method

Manage virtual keyboard input methods:

```bash
# Show input method
tizen-capi-ui-inputmethod-cli show

# Hide input method
tizen-capi-ui-inputmethod-cli hide

# Get input method state
tizen-capi-ui-inputmethod-cli state
```

See [tizen-capi-ui-inputmethod-cli.md](references/tizen-capi-ui-inputmethod-cli.md) for complete documentation.

## UI Input Method Manager

Manage input method lifecycle:

```bash
# Get available input methods
tizen-capi-ui-inputmethod-manager-cli list

# Get active input method
tizen-capi-ui-inputmethod-manager-cli get-active

# Set input method
tizen-capi-ui-inputmethod-manager-cli set-active --id org.tizen.ime
```

See [tizen-capi-ui-inputmethod-manager-cli.md](references/tizen-capi-ui-inputmethod-manager-cli.md) for complete documentation.

## LibFeedback

Control haptic feedback:

```bash
# List feedback types
tizen-libfeedback-cli list-types

# List feedback patterns
tizen-libfeedback-cli list-patterns

# Play feedback pattern
tizen-libfeedback-cli play --type tap --pattern strong
```

See [tizen-libfeedback-cli.md](references/tizen-libfeedback-cli.md) for complete documentation.

## Notification

Manage notifications:

```bash
# Post notification
tizen-notification-cli post --id 1 --title "Hello" --content "World"

# Update notification
tizen-notification-cli update --id 1 --title "Updated"

# Remove notification
tizen-notification-cli remove --id 1
```

See [tizen-notification-cli.md](references/tizen-notification-cli.md) for complete documentation.

## Common use cases

- **Input method editors**: Virtual keyboard management
- **Notification apps**: Display and manage notifications
- **Haptic feedback**: Provide tactile user feedback
- **Form autofill**: Auto-fill form fields

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
---
name: tizen-privilege-push
description: Manages privilege information queries and push notification services. Use when querying application privileges, managing push notification subscriptions, or handling push message delivery.
---

# Tizen Privilege Push

Manages privilege information queries and push notification services.

## Quick reference

**Privilege Info**: Privilege information → `tizen-privilege-info-cli`
**Push**: Push notifications → `tizen-push-cli`

## Privilege Info

Query application privileges:

```bash
# Get privilege name
tizen-privilege-info-cli name http://tizen.org/privilege/internet 3.0 native

# Get privilege description
tizen-privilege-info-cli desc http://tizen.org/privilege/internet

# List privileges
tizen-privilege-info-cli list --locale ko_KR.UTF8 --privilege http://tizen.org/privilege/call --privilege http://tizen.org/privilege/content.read
```

See [tizen-privilege-info-cli.md](references/tizen-privilege-info-cli.md) for complete documentation.

## Push

Manage push notification service:

```bash
# Connect to push service
tizen-push-cli connect --app-id sample.app

# Get registration ID
tizen-push-cli get-registration-id sample.app

# Disconnect
tizen-push-cli disconnect sample.app

# Subscribe
tizen-push-cli subscribe --app-id sample.app

# Unsubscribe
tizen-push-cli unsubscribe --app-id sample.app
```

See [tizen-push-cli.md](references/tizen-push-cli.md) for complete documentation.

## Common use cases

- **Security compliance**: Check application privileges
- **Push notification apps**: Manage push subscriptions
- **Permission management**: Query and manage privileges

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
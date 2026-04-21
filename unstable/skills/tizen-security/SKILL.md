---
name: tizen-security
description: Manages cryptographic keys, certificates, and secure credential storage. Use when working with encryption, key management, certificates, or secure credential storage.
---

# Tizen Security

Manages cryptographic keys, certificates, and secure credential storage.

## Quick reference

**Key Manager**: Key and certificate management → `tizen-key-manager-cli`

## Key Manager

Manage cryptographic keys and certificates:

```bash
# List data info
tizen-key-manager-cli list-data-info

# List keys info
tizen-key-manager-cli list-keys-info

# List certificates info
tizen-key-manager-cli list-certs-info

# Get backend info
tizen-key-manager-cli backend-info sw
tizen-key-manager-cli backend-info tz
```

See [tizen-key-manager-cli.md](references/tizen-key-manager-cli.md) for complete documentation.

## Common use cases

- **Security applications**: Manage encryption keys
- **Credential management**: Secure credential storage
- **Certificate operations**: Handle certificate lifecycle

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
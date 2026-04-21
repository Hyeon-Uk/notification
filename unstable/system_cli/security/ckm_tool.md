# CKM_TOOL

**NAME**

ckm_tool - certificate and key management tool

**SYNOPSIS**

```
ckm_tool [COMMAND] [OPTIONS]
```

**DESCRIPTION**

`ckm_tool` is a command-line utility for managing certificates and keys in Tizen. It provides functionality for certificate installation, removal, and querying operations through the Central Key Manager service.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-i, --install` | Install certificate |
| `-r, --remove` | Remove certificate |
| `-l, --list` | List certificates |

**EXAMPLES**

```
# List certificates
ckm_tool -l

# Install certificate
ckm_tool -i /path/to/certificate.pem

# Remove certificate
ckm_tool -r <cert_alias>
```

**PACKAGE**

Tizen platform tool: platform/core/security/central-key-manager
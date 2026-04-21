# CRYPTSETUP

**NAME**

cryptsetup - manage dm-crypt and LUKS encrypted volumes

**SYNOPSIS**

```
cryptsetup [OPTIONS] <action> <device>
```

**DESCRIPTION**

`cryptsetup` is a command-line utility for managing disk encryption using the dm-crypt kernel module. It supports LUKS (Linux Unified Key Setup) format for encrypted volumes.

**COMMON ACTIONS**

| Action | Description |
|--------|-------------|
| `luksFormat` | Initialize a LUKS partition |
| `luksOpen` | Open a LUKS partition |
| `luksClose` | Close a LUKS partition |
| `luksAddKey` | Add a new key |
| `luksRemoveKey` | Remove a key |
| `status` | Show device status |

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-v, --verbose` | Verbose output |
| `--type TYPE` | Type of encryption (luks, plain) |

**EXAMPLES**

```
# Format a LUKS partition
cryptsetup luksFormat /dev/sda1

# Open encrypted partition
cryptsetup luksOpen /dev/sda1 my_encrypted

# Close encrypted partition
cryptsetup luksClose my_encrypted

# Check status
cryptsetup status my_encrypted
```

**SEE ALSO**

veritysetup(1), dmsetup(1)

**PACKAGE**

External package: cryptsetup
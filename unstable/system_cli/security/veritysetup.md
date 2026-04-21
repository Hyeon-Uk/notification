# VERITYSETUP

**NAME**

veritysetup - manage dm-verity verified volumes

**SYNOPSIS**

```
veritysetup [OPTIONS] <action> <data_device> <hash_device> <root_hash>
```

**DESCRIPTION**

`veritysetup` is a command-line utility for managing dm-verity device-mapper targets. DM-verity provides transparent integrity checking of block devices using a Merkle tree, commonly used for verified boot in embedded systems like Tizen.

**COMMON ACTIONS**

| Action | Description |
|--------|-------------|
| `format` | Create verity hash device |
| `verify` | Verify data device against hash device |
| `open` | Create verity device |
| `close` | Remove verity device |
| `status` | Show device status |

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-v, --verbose` | Verbose output |
| `--hash-alg ALG` | Hash algorithm (sha256, sha512) |
| `--data-block-size SIZE` | Data block size |
| `--hash-block-size SIZE` | Hash block size |

**EXAMPLES**

```
# Format a verity device
veritysetup format /dev/sda1 /dev/sda2

# Open verity device
veritysetup open /dev/sda1 /dev/sda2 verity_dev <root_hash>

# Close verity device
veritysetup close verity_dev

# Verify device integrity
veritysetup verify /dev/sda1 /dev/sda2 <root_hash>
```

**SEE ALSO**

cryptsetup(1), dmsetup(1)

**PACKAGE**

External package: cryptsetup
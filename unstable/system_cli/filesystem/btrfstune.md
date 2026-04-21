# btrfstune - Tune Btrfs filesystem parameters

## NAME

**btrfstune** - Modify tunable filesystem parameters on a Btrfs filesystem.

## SYNOPSIS

```bash
btrfstune [OPTIONS] <device>
```

## DESCRIPTION

**btrfstune** is used to modify various tunable parameters on an unmounted Btrfs filesystem. It can enable or disable features, change UUID, and adjust other filesystem settings.

## OPTIONS

### Feature Options

| Option | Description |
|--------|-------------|
| `-S <0|1>`, `--seeding <0|1>` | Enable/disable seeding flag. |
| `-L`, `--large-metadata` | Enable large metadata blocks. |
| `-r`, `--reserved` | Enable reserved space. |
| `-x`, `--extref` | Enable extended inode references. |
| `-n`, `--no-holes` | Enable no-holes representation. |
| `-N`, `--nodiscard` | Disable discard on mount. |
| `-f`, `--free-space-tree` | Enable free space tree. |
| `-u <uuid>`, `--uuid <uuid>` | Change filesystem UUID. |
| `-U <uuid>`, `--device-uuid <uuid>` | Change device UUID. |
| `-m <uuid>`, `--fs-uuid <uuid>` | Change filesystem UUID. |
| `-M <uuid>`, `--new-uuid <uuid>` | Generate new UUID. |

### Other Options

| Option | Description |
|--------|-------------|
| `-f`, `--force` | Force operation. |
| `-q`, `--quiet` | Quiet mode. |
| `-v`, `--verbose` | Verbose output. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Enable seeding mode

```bash
$ btrfstune -S 1 /dev/sda1
Seeding device enabled on /dev/sda1
```

### Disable seeding mode

```bash
$ btrfstune -S 0 /dev/sda1
Seeding device disabled on /dev/sda1
```

### Enable extended inode references

```bash
$ btrfstune -x /dev/sda1
Extended inode references enabled on /dev/sda1
```

### Enable no-holes feature

```bash
$ btrfstune -n /dev/sda1
No-holes feature enabled on /dev/sda1
```

### Change filesystem UUID

```bash
$ btrfstune -u 12345678-1234-1234-1234-123456789012 /dev/sda1
Filesystem UUID changed on /dev/sda1
```

### Generate new UUID

```bash
$ btrfstune -M /dev/sda1
New UUID generated on /dev/sda1
```

### Enable free space tree

```bash
$ btrfstune -f /dev/sda1
Free space tree enabled on /dev/sda1
```

## NOTES

- The filesystem should be unmounted before running btrfstune.
- Seeding devices are read-only and can be used to create new filesystems.
- UUID changes require a reboot for the kernel to see the new UUID.
- Some features may require a new enough kernel to use.
- Enabling features is permanent and cannot be easily undone.

## SEE ALSO

**btrfs**(8), **mkfs.btrfs**(8), **btrfsck**(8)

## PACKAGE

`btrfs-progs`
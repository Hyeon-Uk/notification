# cgroup_remount - Initialize cgroup directories

## NAME

**cgroup_remount** - Remount or setup cgroup directories for process isolation.

## SYNOPSIS

```bash
cgroup_remount
```

## DESCRIPTION

**cgroup_remount** is a small initialization utility used to remount or setup cgroup directories on the device. It ensures the environment is properly prepared for tools like nsjail to isolate processes securely.

## OPTIONS

This utility takes no options.

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. The cgroup remount operation completed successfully. |
| `1` | Error. Operation failed due to insufficient permissions or filesystem issues. |

## EXAMPLES

### Initialize cgroups

```bash
$ cgroup_remount
```

## NOTES

- Directly accesses `/sys/fs/cgroup`.
- Outputs execution confirmations (e.g., "creation success" or "already created").
- Required for proper operation of containerization tools like nsjail.
- Typically run during system initialization.

## SEE ALSO

**nsjail**(1), **mount**(8)

## PACKAGE

`tizen-tools`
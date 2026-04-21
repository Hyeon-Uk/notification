# xtables-multi

## NAME

xtables-multi - Multi-call binary for iptables and ip6tables utilities

## SYNOPSIS

```
xtables-multi <command> [options]
```

## DESCRIPTION

`xtables-multi` is a multi-call binary that provides the functionality of multiple iptables-related utilities in a single executable. When invoked, it determines which tool to emulate based on the name used to call it or the command specified.

This is an internal implementation detail of the iptables package and users typically interact with it through symbolic links like `iptables`, `ip6tables`, `iptables-save`, `iptables-restore`, etc.

## COMMANDS

| Command | Description |
|---------|-------------|
| `iptables` | IPv4 packet filtering administration |
| `ip6tables` | IPv6 packet filtering administration |
| `iptables-save` | Dump iptables rules |
| `ip6tables-save` | Dump ip6tables rules |
| `iptables-restore` | Restore iptables rules |
| `ip6tables-restore` | Restore ip6tables rules |
| `iptables-apply` | Apply iptables rules with rollback |
| `ip6tables-apply` | Apply ip6tables rules with rollback |

## INTERNAL OPERATION

The `xtables-multi` binary is typically accessed through symbolic links:

```bash
# These are typically symlinks to xtables-multi
/usr/sbin/iptables -> xtables-multi
/usr/sbin/ip6tables -> xtables-multi
/usr/sbin/iptables-save -> xtables-multi
/usr/sbin/iptables-restore -> xtables-multi
```

When called through a symlink, the binary detects which tool to emulate based on the name it was invoked with.

## EXAMPLES

### Direct invocation (not typical usage)

```bash
# Execute iptables command via xtables-multi
xtables-multi iptables -L -n

# Execute ip6tables command
xtables-multi ip6tables -L -n

# Save IPv4 rules
xtables-multi iptables-save > rules.v4

# Restore IPv4 rules
xtables-multi iptables-restore < rules.v4
```

### Typical usage (via symlinks)

```bash
# List iptables rules (actual binary is xtables-multi)
iptables -L -n

# List ip6tables rules
ip6tables -L -n

# Save rules
iptables-save > /etc/iptables/rules.v4

# Restore rules
iptables-restore < /etc/iptables/rules.v4
```

## OPTIONS

Options depend on which command is being executed. See the individual command documentation:

- [iptables](iptables.md) - Options for IPv4 packet filtering
- [ip6tables](ip6tables.md) - Options for IPv6 packet filtering
- [iptables-save](iptables-save.md) - Options for dumping rules
- [iptables-restore](iptables-restore.md) - Options for restoring rules

## BENEFITS OF MULTI-CALL DESIGN

1. **Reduced disk space**: One binary instead of multiple separate executables
2. **Shared code**: Common code is shared between utilities
3. **Smaller memory footprint**: Shared libraries and code pages
4. **Simpler maintenance**: Single source tree for all utilities

## FILES

| File | Description |
|------|-------------|
| `/usr/sbin/xtables-multi` | Multi-call binary location |
| `/usr/sbin/iptables` | Symbolic link to xtables-multi |
| `/usr/sbin/ip6tables` | Symbolic link to xtables-multi |
| `/usr/sbin/iptables-save` | Symbolic link to xtables-multi |
| `/usr/sbin/iptables-restore` | Symbolic link to xtables-multi |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 1 | Failure |
| 2 | Invalid command or option |

## SEE ALSO

- [iptables](iptables.md) - IPv4 packet filtering administration
- [ip6tables](ip6tables.md) - IPv6 packet filtering administration
- [iptables-save](iptables-save.md) - Dump iptables rules
- [iptables-restore](iptables-restore.md) - Restore iptables rules
- [ip6tables-save](ip6tables-save.md) - Dump ip6tables rules
- [ip6tables-restore](ip6tables-restore.md) - Restore ip6tables rules

## PACKAGE

`iptables`
# iptables-save

**NAME**

iptables-save - Dump iptables rules to stdout

**SYNOPSIS**

```bash
iptables-save [OPTIONS]
```

**DESCRIPTION**

`iptables-save` prints the current iptables rules in a format that can be read by iptables-restore. This is useful for backing up firewall configurations.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-t TABLE` | Specify table to dump (default: all tables) |
| `-c`, `--counters` | Include packet/byte counters |
| `-M MODPROBE` | Specify modprobe program path |
| `-h`, `--help` | Display help message |

**EXAMPLES**

1. Save all iptables rules:

```bash
iptables-save > /etc/iptables/rules.v4
```

2. Save with counters:

```bash
iptables-save -c > firewall-backup.rules
```

3. Save specific table:

```bash
iptables-save -t nat > nat-rules.rules
```

**SEE ALSO**

iptables(8), iptables-restore(8), ip6tables-save(8)

**PACKAGE**

iptables
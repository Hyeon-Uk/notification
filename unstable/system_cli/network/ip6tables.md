# ip6tables

**NAME**

ip6tables - Administration tool for IPv6 packet filtering

**SYNOPSIS**

```bash
ip6tables [OPTIONS] -t TABLE -A CHAIN RULE_SPEC
ip6tables [OPTIONS] -t TABLE -D CHAIN RULE_SPEC
ip6tables [OPTIONS] -t TABLE -L [CHAIN]
```

**DESCRIPTION**

`ip6tables` is used to set up, maintain, and inspect the tables of IPv6 packet filter rules in the Linux kernel. It is the IPv6 counterpart to iptables.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-t TABLE` | Specify table (filter, mangle, raw, security) |
| `-A CHAIN` | Append rule to chain |
| `-D CHAIN` | Delete rule from chain |
| `-I CHAIN [NUM]` | Insert rule at position NUM |
| `-R CHAIN NUM` | Replace rule at position NUM |
| `-L [CHAIN]` | List all rules in chain |
| `-F [CHAIN]` | Flush all rules in chain |
| `-Z [CHAIN]` | Zero packet/byte counters |
| `-N CHAIN` | Create new user-defined chain |
| `-X CHAIN` | Delete user-defined chain |
| `-P CHAIN TARGET` | Set policy for chain |
| `-h` | Display help message |

**TABLES**

| Table | Description |
|-------|-------------|
| filter | Default table, for packet filtering |
| mangle | Special packet alteration |
| raw | Configuration for exemptions |
| security | For mandatory access control |

**BUILT-IN CHAINS**

| Chain | Description |
|-------|-------------|
| INPUT | Packets destined for local sockets |
| OUTPUT | Locally generated packets |
| FORWARD | Packets being routed through the system |
| PREROUTING | Packets before routing decision |
| POSTROUTING | Packets after routing decision |

**EXAMPLES**

1. List all IPv6 rules:

```bash
ip6tables -L
```

2. Allow incoming HTTP:

```bash
ip6tables -A INPUT -p tcp --dport 80 -j ACCEPT
```

3. Allow IPv6 ping:

```bash
ip6tables -A INPUT -p icmpv6 -j ACCEPT
```

4. Flush all rules:

```bash
ip6tables -F
```

**FILES**

- `/etc/iptables/rules.v6` - IPv6 rules persistence file
- `/proc/net/ip6_tables_names` - Available tables

**SEE ALSO**

iptables(8), ip6tables-save(8), ip6tables-restore(8)

**PACKAGE**

iptables
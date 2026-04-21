# iptables

**NAME**

iptables - Administration tool for IPv4 packet filtering and NAT

**SYNOPSIS**

```bash
iptables [OPTIONS] -t TABLE -A CHAIN RULE_SPEC
iptables [OPTIONS] -t TABLE -D CHAIN RULE_SPEC
iptables [OPTIONS] -t TABLE -L [CHAIN]
```

**DESCRIPTION**

`iptables` is used to set up, maintain, and inspect the tables of IPv4 packet filter rules in the Linux kernel. Each table contains a number of built-in chains and may also contain user-defined chains.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-t TABLE` | Specify table (filter, nat, mangle, raw, security) |
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
| nat | Network address translation |
| mangle | Special packet alteration |
| raw | Configuration for exemptions |
| security | For mandatory access control |

**CHAIN TARGETS**

| Target | Description |
|--------|-------------|
| ACCEPT | Accept the packet |
| DROP | Drop the packet |
| REJECT | Reject packet and return notification |
| LOG | Log packet to syslog |
| DNAT | Destination NAT |
| SNAT | Source NAT |
| MASQUERADE | Dynamic source NAT |

**EXAMPLES**

1. List all rules:

```bash
iptables -L
```

2. Allow incoming SSH connections:

```bash
iptables -A INPUT -p tcp --dport 22 -j ACCEPT
```

3. Block an IP address:

```bash
iptables -A INPUT -s 192.168.1.100 -j DROP
```

4. Enable NAT masquerading:

```bash
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
```

5. Flush all rules:

```bash
iptables -F
```

**FILES**

- `/etc/iptables/rules.v4` - IPv4 rules persistence file
- `/proc/net/ip_tables_names` - Available tables

**SEE ALSO**

ip6tables(8), iptables-save(8), iptables-restore(8)

**PACKAGE**

iptables
# connmanctl

## NAME

connmanctl - Command line interface for Connection Manager

## SYNOPSIS

```
connmanctl [options] [command [arguments]]
```

## DESCRIPTION

`connmanctl` is a command line interface for ConnMan (Connection Manager), a daemon for managing network connections on Linux systems. It provides both interactive and non-interactive modes for configuring WiFi, Ethernet, Bluetooth, and other network technologies.

ConnMan is designed to be lightweight and fast, making it suitable for embedded devices and Tizen platform.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message |
| `-v`, `--version` | Show version information |

## COMMANDS

### Global Commands

| Command | Description |
|---------|-------------|
| `help` | Show help for available commands |
| `exit` | Exit interactive mode |
| `quit` | Same as exit |

### Technology Commands

| Command | Description |
|---------|-------------|
| `technologies` | List all available technologies (wifi, ethernet, bluetooth, etc.) |
| `enable <technology>` | Enable a technology (e.g., `enable wifi`) |
| `disable <technology>` | Disable a technology |
| `tether <technology> on\|off` | Enable/disable tethering |
| `tether <technology> <ssid> <psk>` | Setup WiFi tethering with SSID and passphrase |

### Service Commands

| Command | Description |
|---------|-------------|
| `services` | List all available services (networks) |
| `services <pattern>` | Filter services by pattern |
| `connect <service>` | Connect to a service |
| `disconnect <service>` | Disconnect from a service |
| `remove <service>` | Remove a service (clear credentials) |

### WiFi Commands

| Command | Description |
|---------|-------------|
| `scan <technology>` | Scan for networks (e.g., `scan wifi`) |
| `agent on` | Enable the agent for authentication |
| `agent off` | Disable the agent |
| `services` | Display available WiFi networks after scan |

### Configuration Commands

| Command | Description |
|---------|-------------|
| `config <service> ipv4 <method> [address] [netmask] [gateway]` | Configure IPv4 settings |
| `config <service> ipv6 <method> [address] [prefixlen] [gateway]` | Configure IPv6 settings |
| `config <service> nameservers <dns1> [dns2] ...` | Configure DNS servers |
| `config <service> domains <domain1> [domain2] ...` | Configure search domains |
| `config <service> proxy <method> [url]` | Configure proxy settings |
| `config <service> autoconnect on\|off` | Enable/disable auto-connect |

### Information Commands

| Command | Description |
|---------|-------------|
| `state` | Show connection state |
| `services` | List services with their states |
| `peers` | List peer (P2P) services |
| `sessions` | List sessions |
| `providers` | List providers |
| `vpnconnections` | List VPN connections |

## EXAMPLES

### Interactive Mode

Enter interactive mode:
```bash
connmanctl
```

In interactive mode, you can enter commands directly without the `connmanctl` prefix:
```
connmanctl> help
connmanctl> technologies
connmanctl> exit
```

### Scan and Connect to WiFi

Non-interactive approach:
```bash
# Enable WiFi
connmanctl enable wifi

# Scan for networks
connmanctl scan wifi

# List available services
connmanctl services

# Connect to a network (requires agent for password)
connmanctl agent on
connmanctl connect wifi_XXXXXXXXXXXX_managed_psk
```

Interactive approach:
```
connmanctl> agent on
Agent registered
connmanctl> scan wifi
Scan completed
connmanctl> services
wifi_1234567890ab_managed_psk    MyWiFi    ready
connmanctl> connect wifi_1234567890ab_managed_psk
Passphrase? mypassword
Connected
connmanctl> quit
```

### Configure Static IP

```bash
# Set static IPv4 configuration
connmanctl config wifi_XXXXXXXXXXXX_managed_psk ipv4 manual 192.168.1.100 255.255.255.0 192.168.1.1

# Set DNS servers
connmanctl config wifi_XXXXXXXXXXXX_managed_psk nameservers 8.8.8.8 8.8.4.4

# Enable DHCP (dynamic)
connmanctl config wifi_XXXXXXXXXXXX_managed_psk ipv4 dhcp
```

### Enable/Disable Technologies

```bash
# Enable WiFi
connmanctl enable wifi

# Disable WiFi
connmanctl disable wifi

# Enable Ethernet
connmanctl enable ethernet

# Enable Bluetooth
connmanctl enable bluetooth
```

### Tethering (Hotspot)

```bash
# Enable WiFi tethering with SSID and password
connmanctl tether wifi MyHotspot MyPassword on

# Disable tethering
connmanctl tether wifi off
```

### Check Connection State

```bash
connmanctl state
```

### Disconnect and Forget Network

```bash
# Disconnect from a service
connmanctl disconnect wifi_XXXXXXXXXXXX_managed_psk

# Remove (forget) a service
connmanctl remove wifi_XXXXXXXXXXXX_managed_psk
```

## INTERACTIVE MODE

When run without arguments, `connmanctl` starts in interactive mode:
```bash
$ connmanctl
connmanctl> 
```

This provides a shell-like environment for managing connections. Commands in interactive mode do not require the `connmanctl` prefix.

## FILES

| File | Description |
|------|-------------|
| `/var/lib/connman/` | ConnMan storage directory |
| `/var/lib/connman/settings` | Main settings file |
| `/var/lib/connman/*/settings` | Per-service settings |
| `/etc/connman/main.conf` | Main configuration file |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 1 | General error |
| 2 | Command not found |

## SEE ALSO

- [wifi_connect_tool](wifi_connect_tool.md) - Tizen WiFi connection tool
- [rndis.sh](rndis.sh.md) - RNDIS network configuration script

## PACKAGE

`connman`
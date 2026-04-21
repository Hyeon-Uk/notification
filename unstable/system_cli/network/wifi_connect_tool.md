# wifi_connect_tool

## NAME

wifi_connect_tool - Tizen WiFi connection management tool

## SYNOPSIS

```
wifi_connect_tool [options] [command]
```

## DESCRIPTION

`wifi_connect_tool` is a Tizen-specific command line tool for managing WiFi connections. It provides functionality to scan for available networks, connect to WiFi access points, disconnect from networks, and display WiFi connection status.

This tool is designed specifically for Tizen platform and interacts with the Tizen WiFi service through D-Bus.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message |
| `-v`, `--version` | Show version information |

## COMMANDS

| Command | Description |
|---------|-------------|
| `scan` | Scan for available WiFi networks |
| `connect <ssid> <password>` | Connect to a WiFi network with SSID and password |
| `connect <ssid>` | Connect to an open WiFi network |
| `disconnect` | Disconnect from current WiFi network |
| `status` | Show current WiFi connection status |
| `list` | List saved/known networks |
| `remove <ssid>` | Remove a saved network |
| `info` | Show detailed WiFi information |

## EXAMPLES

### Scan for available networks

```bash
wifi_connect_tool scan
```

### Connect to a secured network

```bash
# Connect to WPA/WPA2 network
wifi_connect_tool connect MyWiFi MyPassword
```

### Connect to an open network

```bash
# Connect to open network (no password)
wifi_connect_tool connect OpenWiFi
```

### Check connection status

```bash
wifi_connect_tool status
```

### Disconnect from current network

```bash
wifi_connect_tool disconnect
```

### List saved networks

```bash
wifi_connect_tool list
```

### Remove a saved network

```bash
wifi_connect_tool remove MyWiFi
```

### Show detailed WiFi information

```bash
wifi_connect_tool info
```

### Complete workflow example

```bash
# 1. Scan for networks
wifi_connect_tool scan

# 2. Connect to a network
wifi_connect_tool connect MyHomeWiFi MyPassword123

# 3. Verify connection
wifi_connect_tool status

# 4. Check detailed info
wifi_connect_tool info

# 5. Disconnect when done
wifi_connect_tool disconnect
```

## OUTPUT FORMAT

### Scan output example
```
SSID                    BSSID               Signal  Security
------------------------------------------------------------
MyWiFi                  AA:BB:CC:DD:EE:FF   85%     WPA2
GuestNetwork            AA:BB:CC:DD:EE:EE   72%     Open
NeighborWiFi            11:22:33:44:55:66   45%     WPA2
```

### Status output example
```
State: Connected
SSID: MyWiFi
BSSID: AA:BB:CC:DD:EE:FF
Signal: 85%
Security: WPA2-PSK
IP Address: 192.168.1.100
Frequency: 2.4 GHz
```

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 1 | General error |
| 2 | Connection failed |
| 3 | Network not found |
| 4 | Authentication failed |

## FILES

| File | Description |
|------|-------------|
| `/etc/wifi/` | WiFi configuration directory |
| `/var/run/wifi/` | Runtime WiFi state files |
| `/opt/usr/share/wifi/` | WiFi related resources |

## TROUBLESHOOTING

### Network not found

```bash
# Rescan for networks
wifi_connect_tool scan

# Check if WiFi is enabled
wifi_connect_tool status
```

### Connection failed

```bash
# Check password and try again
wifi_connect_tool connect MyWiFi correct_password

# Check WiFi signal strength
wifi_connect_tool scan | grep MyWiFi
```

### Authentication failed

```bash
# Verify correct password
# Check if MAC filtering is enabled on the AP
# Try removing saved network and reconnecting
wifi_connect_tool remove MyWiFi
wifi_connect_tool connect MyWiFi MyPassword
```

## SEE ALSO

- [connmanctl](connmanctl.md) - Connection Manager command line interface
- [rndis.sh](rndis.sh.md) - RNDIS USB networking script

## PACKAGE

`wifi-tools` (Tizen package name may vary)
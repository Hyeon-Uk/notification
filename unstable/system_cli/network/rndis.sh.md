# rndis.sh

## NAME

rndis.sh - RNDIS (Remote Network Driver Interface Specification) setup script for USB networking

## SYNOPSIS

```
rndis.sh [start|stop|status]
```

## DESCRIPTION

`rndis.sh` is a shell script for configuring RNDIS (Remote Network Driver Interface Specification) USB networking on Tizen devices. RNDIS allows a Tizen device to appear as a network interface when connected via USB to a host computer, enabling USB tethering functionality.

RNDIS provides a virtual Ethernet link over USB, allowing the host computer to share the Tizen device's network connection or access device services.

## OPTIONS

| Option | Description |
|--------|-------------|
| `start` | Start RNDIS service and configure USB networking |
| `stop` | Stop RNDIS service and cleanup network configuration |
| `status` | Show current RNDIS status |

## EXAMPLES

### Start RNDIS networking

```bash
# Start RNDIS service
rndis.sh start
```

### Stop RNDIS networking

```bash
# Stop RNDIS service
rndis.sh stop
```

### Check RNDIS status

```bash
# Check if RNDIS is active
rndis.sh status
```

### Typical usage workflow

```bash
# Connect device via USB

# Start RNDIS on Tizen device
rndis.sh start

# On host PC, the device should now appear as a network interface
# e.g., usb0 or enp0s20u1

# Configure IP on host if not automatic
# Host PC:
# sudo ip addr add 192.168.42.2/24 dev usb0
# sudo ip link set usb0 up

# Test connectivity
ping 192.168.42.1  # Device IP

# When done, stop RNDIS
rndis.sh stop
```

## HOW IT WORKS

1. When started, the script:
   - Loads necessary kernel modules (g_rndis, usb_f_rndis)
   - Configures USB gadget subsystem
   - Creates a virtual network interface (typically `usb0`)
   - Assigns IP address to the interface
   - Enables IP forwarding and NAT for internet sharing

2. When stopped, the script:
   - Removes IP configuration
   - Unloads kernel modules
   - Cleans up USB gadget configuration

## NETWORK CONFIGURATION

RNDIS typically uses the following default network configuration:

| Parameter | Typical Value |
|-----------|---------------|
| Device IP | 192.168.42.1 |
| Subnet | 255.255.255.0 (192.168.42.0/24) |
| Host IP | 192.168.42.2 (assigned by host) |
| DNS | Often forwarded from device's connection |

## FILES

| File | Description |
|------|-------------|
| `/usr/bin/rndis.sh` | RNDIS setup script |
| `/sys/class/net/usb0/` | USB network interface |
| `/sys/kernel/config/usb_gadget/` | USB gadget configuration |
| `/etc/sysconfig/rndis.conf` | Configuration file (if applicable) |

## PREREQUISITES

- USB cable connection to host PC
- Kernel support for USB gadget and RNDIS
- Root privileges

## TROUBLESHOOTING

### Device not detected on host

```bash
# Check if RNDIS is running
rndis.sh status

# Verify USB connection
ls /sys/class/net/usb0

# Check kernel modules
lsmod | grep rndis
```

### No network connectivity

```bash
# Check IP configuration
ip addr show usb0

# Check routing
ip route

# Verify IP forwarding
cat /proc/sys/net/ipv4/ip_forward
```

### USB gadget errors

```bash
# Check kernel logs
dmesg | grep -i "gadget\|rndis"

# Check USB configuration
ls -la /sys/kernel/config/usb_gadget/
```

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 1 | General error (missing modules, permission denied, etc.) |
| 2 | Invalid argument |

## SEE ALSO

- [connmanctl](connmanctl.md) - Connection Manager command line interface
- [wifi_connect_tool](wifi_connect_tool.md) - WiFi connection tool

## PACKAGE

`usb-networking` (Tizen package name may vary)
# nbd-client

## NAME

nbd-client - Connect to a Network Block Device server

## SYNOPSIS

```
nbd-client [options] host [port] nbd-device
nbd-client -disconnect nbd-device
nbd-client -check nbd-device
```

## DESCRIPTION

`nbd-client` is a client for the Linux Network Block Device (NBD) protocol. It allows a Linux system to use a remote block device served by an NBD server as if it were a local block device. This is useful for accessing remote storage over the network.

The NBD protocol enables a client machine to access block devices stored on a remote server. The client kernel module presents the remote device as a local block device (e.g., `/dev/nbd0`).

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message |
| `-V`, `--version` | Show version information |
| `-d`, `--disconnect` | Disconnect the specified NBD device |
| `-c`, `--check` | Check whether the specified NBD device is connected |
| `-n`, `--nofork` | Do not fork into background |
| `-s`, `--swap` | The NBD device will be used as swap. Use with caution |
| `-m`, `--multi` | Allow multiple connections to the server |
| `-C`, `--connections` | Number of connections to use (default: 1) |
| `-p`, `--persist` | Reconnect to the server if connection is lost |
| `-S`, `--size` | Set the size of the NBD device (bytes) |
| `-b`, `--block-size` | Set the block size (bytes) |
| `-t`, `--timeout` | Set the timeout in seconds |
| `-a`, `--attempt` | Maximum number of connection attempts |

## EXAMPLES

### Connect to an NBD server

```bash
# Basic connection to NBD server
nbd-client 192.168.1.100 /dev/nbd0

# Connect with specific port
nbd-client 192.168.1.100 10809 /dev/nbd0
```

### Connect with multiple connections

```bash
# Use 4 connections for better performance
nbd-client -C 4 192.168.1.100 /dev/nbd0
```

### Connect in persistent mode

```bash
# Auto-reconnect on connection failure
nbd-client -p 192.168.1.100 /dev/nbd0
```

### Check if NBD device is connected

```bash
nbd-client -c /dev/nbd0
echo $?  # Returns 0 if connected, 1 if not
```

### Disconnect an NBD device

```bash
# Disconnect NBD device
nbd-client -d /dev/nbd0
```

### Use NBD device as swap

```bash
# Connect NBD device for swap usage
nbd-client -s 192.168.1.100 /dev/nbd0

# Setup swap area
mkswap /dev/nbd0
swapon /dev/nbd0
```

### Full workflow example

```bash
# Load NBD kernel module
modprobe nbd

# Connect to NBD server
nbd-client 192.168.1.100 /dev/nbd0

# Check connection
nbd-client -c /dev/nbd0
# Output: Connected

# Create filesystem
mkfs.ext4 /dev/nbd0

# Mount the device
mount /dev/nbd0 /mnt/nbd

# Use the device...

# Unmount when done
umount /mnt/nbd

# Disconnect
nbd-client -d /dev/nbd0
```

### Set timeout and retry attempts

```bash
# Set 30 second timeout and 5 retry attempts
nbd-client -t 30 -a 5 192.168.1.100 /dev/nbd0
```

## NBD DEVICES

NBD devices are typically named `/dev/nbdX` where X is a number from 0 to 15 (or higher depending on kernel configuration). These devices are created when the `nbd` kernel module is loaded.

```bash
# Load NBD module with support for 16 devices
modprobe nbd nbds_max=16

# List available NBD devices
ls -la /dev/nbd*
```

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 1 | Failure (connection error, invalid device, etc.) |
| 2 | Device not connected (when using `-c` option) |

## FILES

| File | Description |
|------|-------------|
| `/dev/nbd*` | NBD block device files |
| `/proc/partitions` | Contains information about NBD partitions |

## SEE ALSO

- [nbd-server](https://manpages.debian.org/nbd-server) - NBD server daemon
- [modprobe](../kernel_module/modprobe.md) - Load kernel modules

## PACKAGE

`nbd`
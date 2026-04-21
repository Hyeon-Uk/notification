# devicectl - Device control command-line tool

## NAME

**devicectl** - Control various system devices through the device daemon.

## SYNOPSIS

```bash
devicectl <device_name> <action> [option]
devicectl --help
```

## DESCRIPTION

**devicectl** is a command-line tool for controlling various system devices through the Tizen device daemon (deviced). It provides an interface to start, stop, and configure hardware devices such as display, LED, USB, power, and external connectors.

## DEVICES

| Device Name | Description |
|-------------|-------------|
| `core` | Core system device |
| `display` | Display device |
| `led` | LED indicator device |
| `pass` | Pass-through device |
| `usb` | USB device |
| `extcon` | External connector device |
| `power` | Power management device |
| `usb-host-test` | USB host test device |

## COMMANDS

### All Devices

| Command | Description |
|---------|-------------|
| `devicectl <device> start` | Start the specified device. |
| `devicectl <device> stop` | Stop the specified device. |

### Display Commands

| Command | Description |
|---------|-------------|
| `devicectl display dumpmode [on\|off]` | Set display dump mode. |
| `devicectl display state [lcdon\|lcdoff]` | Change display state. |
| `devicectl display savelog` | Save display log. |

### LED Commands

| Command | Description |
|---------|-------------|
| `devicectl led dumpmode [on\|off]` | Set LED dump mode. |

### USB Commands

| Command | Description |
|---------|-------------|
| `devicectl usb set [sdb\|ssh]` | Set USB mode to SDB or SSH. |
| `devicectl usb unset [sdb\|ssh]` | Unset USB mode. |

### Core Commands

| Command | Description |
|---------|-------------|
| `devicectl core dbusname` | Save D-Bus connection names. |
| `devicectl core devicelist` | Print device list to dlog. |

### External Connector Commands

| Command | Description |
|---------|-------------|
| `devicectl extcon enable [USB\|HEADPHONE\|HDMI\|DOCK]` | Enable external connector. |
| `devicectl extcon disable [USB\|HEADPHONE\|HDMI\|DOCK]` | Disable external connector. |

### Power Commands

| Command | Description |
|---------|-------------|
| `devicectl power suspend <reason>` | Suspend the device with reason code. |
| `devicectl power resume <reason>` | Resume the device with reason code. |
| `devicectl power off` | Power off the device. |
| `devicectl power reboot` | Reboot the device. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `-22` (EINVAL) | Invalid arguments. |
| `-75` (EBADMSG) | D-Bus communication error. |
| `-12` (ENOMEM) | Memory allocation error. |

## EXAMPLES

### Start/Stop devices

```bash
$ devicectl display start
Start display device.

$ devicectl display stop
Stop display device.
```

### Control display state

```bash
$ devicectl display state lcdon
display (state lcdon)!

$ devicectl display state lcdoff
display (state lcdoff)!
```

### Set USB mode

```bash
$ devicectl usb set sdb
Start usb device.

$ devicectl usb unset sdb
```

### Enable external connector

```bash
$ devicectl extcon enable USB
Enable USB device.
```

### Power management

```bash
$ devicectl power suspend 1
Power suspend device.

$ devicectl power resume 1
Power resume device.

$ devicectl power reboot
Power reboot device.
```

### Save D-Bus names

```bash
$ devicectl core dbusname
Save dbus name.
N connections.
  PID    NAME    COMMAND
  ...
```

## NOTES

- This tool communicates with the deviced daemon through D-Bus.
- The deviced daemon must be running for this tool to work.
- Some commands require root privileges.
- Use `--help` to see all available commands and options.

## SEE ALSO

**deviced**(8), **systemctl**(1)

## PACKAGE

`deviced`
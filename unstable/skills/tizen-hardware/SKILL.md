---
name: tizen-hardware
description: Accesses device hardware capabilities including sensors and screen mirroring. Use when working with device sensors, hardware capabilities, or screen mirroring functionality.
---

# Tizen Hardware

Accesses device hardware capabilities including sensors and screen mirroring.

## Quick reference

**Sensor**: Sensor data access → `tizen-sensor-cli`
**Screen Mirroring**: Screen mirroring → `tizen-screen-mirroring-cli`

## Sensor

Access device sensor data:

```bash
# List available sensors
tizen-sensor-cli list

# Get sensor data
tizen-sensor-cli get --type accelerometer

# Monitor sensor
tizen-sensor-cli monitor --type gyroscope
```

See [tizen-sensor-cli.md](references/tizen-sensor-cli.md) for complete documentation.

## Screen Mirroring

Manage screen mirroring:

```bash
# Start mirroring
tizen-screen-mirroring-cli start --address 192.168.1.100

# Stop mirroring
tizen-screen-mirroring-cli stop

# Get status
tizen-screen-mirroring-cli status
```

See [tizen-screen-mirroring-cli.md](references/tizen-screen-mirroring-cli.md) for complete documentation.

## Common use cases

- **Fitness apps**: Track accelerometer and gyroscope data
- **Screen casting**: Mirror device screen to external display
- **Sensor monitoring**: Monitor hardware sensors in real-time

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
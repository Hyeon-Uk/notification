# TIZEN-SENSOR-CLI(1)

## NAME
**tizen-sensor-cli** - Query Tizen sensor support, metadata, and sample data.

## SYNOPSIS
**tizen-sensor-cli** *COMMAND* [*OPTIONS*]  
**tizen-sensor-cli** **-h** | **--help** | **help**  
**tizen-sensor-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-sensor-cli** provides command-line access to the public Tizen
sensor APIs from `sensor.h`.

The tool supports:
- listing supported public sensor types
- checking whether a sensor type is supported
- inspecting default sensor metadata and capability values
- reading one synchronous sample from a sensor listener

Some sensor types, especially `hrm*` and `human-*`, may require
`http://tizen.org/privilege/healthinfo`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list** | None | Standard | Lists supported public sensor types using `sensor_is_supported()`. |
| **list** | `--sensor <NAME>` | Depends on sensor | Lists sensor handles for a given type using `sensor_get_sensor_list()`. |
| **is-supported** | `--sensor <NAME>` | Standard | Prints whether the requested sensor type is supported. |
| **info** | `--sensor <NAME>` | Depends on sensor | Prints default sensor name, vendor, URI, type, wake-up flag, range, resolution, interval, FIFO, and batch values. |
| **read** | `--sensor <NAME>` | Depends on sensor | Starts a listener, reads one sample with `sensor_listener_read_data()`, and prints the values. |

Common sensor names: `accelerometer`, `gravity`,
`linear-acceleration`, `magnetic`, `rotation-vector`, `orientation`,
`gyroscope`, `light`, `proximity`, `pressure`, `ultraviolet`,
`temperature`, `humidity`, `hrm`

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or sensor API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-sensor-cli`

## AUTHOR
Part of the **Tizen Tools** project.

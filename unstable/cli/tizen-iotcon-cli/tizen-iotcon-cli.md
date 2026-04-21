# TIZEN-IOTCON-CLI(1)

## NAME
**tizen-iotcon-cli** - Discover OCF resources and issue one-shot IoTCon
client requests.

## SYNOPSIS
**tizen-iotcon-cli** *COMMAND* [*OPTIONS*]  
**tizen-iotcon-cli** **-h** | **--help** | **help**  
**tizen-iotcon-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-iotcon-cli** provides command-line access to the public Tizen IoTCon
client APIs from `iotcon.h` and `iotcon-client.h`.

The tool supports:
- discovering resources, devices, and platform information
- sending one-shot `GET`, `PUT`, `POST`, and `DELETE` requests
- registering resource observation
- inspecting the current IoTCon timeout and polling interval

For convenience, `--host` accepts either a full CoAP URL such as
`coap://192.168.0.10:5683` or a bare address, in which case the CLI prefixes
`coap://` automatically. Resource URIs are normalized to start with `/`.

Most runtime commands require the device feature
`http://tizen.org/feature/iot.ocf` and the privileges
`http://tizen.org/privilege/internet` and
`http://tizen.org/privilege/network.get`.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **find-resource** | `[--host <ADDR>] [--wait <SECS>]` | Discovers resources for the specified wait interval. |
| **find-device** | `[--host <ADDR>] [--wait <SECS>]` | Discovers device information for the specified wait interval. |
| **find-platform** | `[--host <ADDR>] [--wait <SECS>]` | Discovers platform information for the specified wait interval. |
| **get** | `--host <ADDR> --uri <URI>` | Sends a one-shot `GET` request to the target resource. |
| **put** | `--host <ADDR> --uri <URI> --data <JSON>` | Sends a `PUT` request with an inline JSON object payload. |
| **post** | `--host <ADDR> --uri <URI> --data <JSON>` | Sends a `POST` request with an inline JSON object payload. |
| **delete** | `--host <ADDR> --uri <URI>` | Sends a `DELETE` request to the target resource. |
| **observe** | `--host <ADDR> --uri <URI>` | Registers observation on the target resource and prints notifications. |
| **status** | None | Prints the current IoTCon asynchronous timeout and polling interval. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, IoTCon initialization failure, unsupported feature,
or command failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-iotcon-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-NSD-CLI(1)

## NAME
**tizen-nsd-cli** - Inspect DNS-SD service handles exposed by Tizen NSD APIs.

## SYNOPSIS
**tizen-nsd-cli** *COMMAND* [*OPTIONS*]  
**tizen-nsd-cli** **-h** | **--help** | **help**  
**tizen-nsd-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-nsd-cli** provides command-line access to the public DNS-SD APIs from
`dns-sd.h`.

The tool focuses on safe, one-shot handle inspection commands that work without
running a long-lived event loop:
- creating and printing local service templates
- attaching one TXT record to a local service template
- creating and printing remote service templates
- listing documented browse state enum values

Some commands can report a documented runtime fallback when DNS-SD is not
supported or the background daemon is unavailable on the current device or
emulator.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **local-template** | `--type <SERVICE_TYPE> --name <NAME> --port <PORT> [--interface <IFACE>]` | Standard | Creates a local service handle, sets its public fields, and prints the resolved type, name, port, and interface. |
| **txt-template** | `--type <SERVICE_TYPE> --name <NAME> --port <PORT> --key <KEY> [--value <VALUE>]` | Standard | Creates a local service handle, adds one TXT record, and prints the encoded TXT payload length. |
| **remote-template** | `--type <SERVICE_TYPE> --name <NAME> [--interface <IFACE>]` | Standard | Creates a remote service handle and prints the resolved type, name, and interface. |
| **list-states** | None | Standard | Lists the browse state enum values documented in `dns-sd.h`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-nsd-cli`

## AUTHOR
Part of the **Tizen Tools** project.

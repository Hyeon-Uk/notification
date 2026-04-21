# TIZEN-MTP-CLI(1)

## NAME
**tizen-mtp-cli** - Inspect connected MTP devices, storages, and object
handles through the public Tizen MTP API.

## SYNOPSIS
**tizen-mtp-cli** *COMMAND* [*OPTIONS*]  
**tizen-mtp-cli** **-h** | **--help** | **help**  
**tizen-mtp-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-mtp-cli** provides command-line access to the native Tizen MTP
APIs from `mtp.h`.

The tool supports:
- enumerating currently connected MTP devices with `mtp_get_devices()`
- reading device identity strings with `mtp_deviceinfo_get_*()`
- enumerating storage handles with `mtp_get_storages()`
- reading storage metadata with `mtp_storageinfo_get_*()`
- enumerating object handles within a storage with
  `mtp_get_object_handles()`

MTP availability depends on the device runtime, USB state, and platform
support. When the subsystem is unsupported, inactive, or denied by the
runtime, the CLI prints a documented warning instead of reporting a test
failure.

Commands that interact with device or host storage may require
`http://tizen.org/privilege/mediastorage` or
`http://tizen.org/privilege/externalstorage`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list-devices** | None | Standard | Lists currently visible MTP device handles returned by `mtp_get_devices()`. |
| **device-info** | `[--device-index <N>]` | Standard | Prints manufacturer, model, serial number, and device version for the selected device. |
| **list-storages** | `[--device-index <N>]` | Standard | Lists storage handles for the selected device via `mtp_get_storages()`. |
| **storage-info** | `--storage-index <N> [--device-index <N>]` | Standard | Prints description, volume identifier, storage type, free space, and maximum capacity for the selected storage. |
| **list-object-handles** | `--storage-index <N> [--device-index <N>] [--parent <HANDLE>] [--file-type <TYPE>]` | Standard | Lists object handles for a storage and optional parent handle. Supported file-type filters are `all`, `all-image`, `folder`, `jpeg`, `png`, `mp3`, `mp4`, `text`, and `heif`. |

## EXIT STATUS
**0**: Success, or a documented runtime warning such as unsupported MTP,
inactive transport, or no connected device.  
**1**: Invalid arguments or an unexpected MTP API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-mtp-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-LIBSTORAGE64-CLI(1)

## NAME
**tizen-libstorage64-cli** - Inspect Tizen storage devices and 64-bit storage sizes.

## SYNOPSIS
**tizen-libstorage64-cli** *COMMAND* [*OPTIONS*]  
**tizen-libstorage64-cli** **-h** | **--help** | **help**  
**tizen-libstorage64-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-libstorage64-cli** provides command-line access to the native Tizen
Storage APIs from `storage.h` and `storage-expand.h`.

The tool supports:
- listing supported storage devices
- showing type, state, root directory, and 64-bit space for a storage ID
- resolving a storage root directory
- resolving a public media directory path by storage ID and directory type
- reading `statvfs64`-backed internal and external storage size information

Directory type arguments can be provided as either numeric enum values or
symbolic names such as `images`, `downloads`, or
`STORAGE_DIRECTORY_SYSTEM_RINGTONES`.

In a plain `sdb shell` context, internal-storage path APIs can be restricted by
`libstorage`, so `list`, `root-dir`, or `directory` may report a documented
shell-context limitation instead of exposing internal paths.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **list** | None | Lists supported storage IDs with type, state, and mount path. |
| **info** | `--id <ID>` | Shows type, state, root directory, total space, and available space for the selected storage. |
| **root-dir** | `--id <ID>` | Prints the absolute root directory for the selected storage. |
| **directory** | `--id <ID> --type <ID\|NAME>` | Prints the absolute path for a public storage directory such as `images` or `downloads`. |
| **internal** | None | Prints internal-storage `statvfs64` details and derived byte totals. |
| **external** | None | Prints primary external-storage `statvfs64` details when supported. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments.  
**<0**: A Storage API error code returned by the underlying Tizen API.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-libstorage64-cli`

## AUTHOR
Part of the **Tizen Tools** project.

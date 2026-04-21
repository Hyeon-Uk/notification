# TIZEN-URL-DOWNLOAD-CLI(1)

## NAME
**tizen-url-download-cli** - inspect public URL download request settings
through `download.h`.

## SYNOPSIS
**tizen-url-download-cli** *COMMAND* [*OPTIONS*]  
**tizen-url-download-cli** **-h** | **--help** | **help**  
**tizen-url-download-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-url-download-cli** provides command-line access to the public
URL download APIs from `download.h`.

Each invocation creates a temporary download handle when required,
applies the requested configuration change, reads the public state back,
prints the result, and releases the handle before exiting.

The tool focuses on finite getter/setter and header-management workflows
that can be validated without leaving background download jobs running.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **create-state** | None | Standard | Creates a download handle and prints its initial `READY`-style state. |
| **roundtrip-url** | `--url <URL>` | Standard | Sets the request URL and reads it back with `download_get_url()`. |
| **roundtrip-destination** | `--path <PATH>` | Standard | Sets the destination directory and reads it back with `download_get_destination()`. |
| **roundtrip-file-name** | `--name <NAME>` | Standard | Sets the output file name and reads it back with `download_get_file_name()`. |
| **roundtrip-network** | `--type <data\|wifi\|wifi-direct\|all>` | Standard | Sets the allowed network type and reads it back with `download_get_network_type()`. |
| **roundtrip-auto-download** | `--enabled <true\|false>` | Standard | Sets the auto-download flag and reads it back with `download_get_auto_download()`. |
| **notification-title-roundtrip** | `--title <TITLE>` | Standard | Sets the notification title and reads it back with `download_get_notification_title()`. |
| **notification-description-roundtrip** | `--description <TEXT>` | Standard | Sets the notification description and reads it back with `download_get_notification_description()`. |
| **header-roundtrip** | `--field <FIELD> --value <VALUE>` | Standard | Adds an HTTP header field and reads the same field back. |
| **remove-header** | `--field <FIELD> --value <VALUE>` | Standard | Adds an HTTP header field, removes it, and verifies that the field is no longer present. |
| **roundtrip-cache** | `--enabled <true\|false>` | Standard | Sets the cache flag for the download service and reads it back with `download_get_cache()`. |
| **reset-cache** | None | Standard | Resets the cache configuration to the platform default with `download_reset_cache()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or a URL download API call failed.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-url-download-cli`

## AUTHOR
Part of the **Tizen Tools** project.

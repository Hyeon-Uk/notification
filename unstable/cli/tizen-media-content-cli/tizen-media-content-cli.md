# TIZEN-MEDIA-CONTENT-CLI(1)

## NAME
**tizen-media-content-cli** - Query indexed media database state and
request media rescans on Tizen.

## SYNOPSIS
**tizen-media-content-cli** *COMMAND* [*OPTIONS*]  
**tizen-media-content-cli** **-h** | **--help** | **help**  
**tizen-media-content-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-media-content-cli** provides command-line access to public
functions from `media_content.h`, `media_info.h`, and `media_folder.h`
in `capi-content-media-content`.

The tool focuses on finite shell-friendly operations:
- querying indexed media and folder counts
- requesting file or folder rescans
- canceling a pending folder scan request
- looking up media metadata by indexed path

Commands that depend on the media content service, DB access, or storage
privileges may print `[WARN]` and exit successfully when run from a
plain `sdb shell` session on an emulator or headless target.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **count** | None | Standard | Prints the number of media items currently indexed in the media database. |
| **folder-count** | None | Standard | Prints the number of media folders currently indexed in the media database. |
| **scan-file** | `<PATH>` | `content.write` and storage privilege | Requests a rescan of a specific file path. |
| **scan-folder** | `<PATH> [--recursive]` | `content.write` and storage privilege | Requests a folder rescan. `--recursive` includes subdirectories. |
| **cancel-scan-folder** | `<PATH>` | `content.write` and storage privilege | Requests cancellation of a pending folder scan for the path. |
| **path-info** | `<PATH>` | storage privilege | Prints indexed metadata such as media ID, title, file path, MIME type, size, and media type for a path. |

## EXIT STATUS
**0**: Success, or a documented environment fallback reported as
`[WARN]`.  
**1**: Invalid arguments or an unexpected CLI/runtime failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-media-content-cli`

## AUTHOR
Part of the **Tizen Tools** project.

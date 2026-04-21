# TIZEN-THUMBNAIL-UTIL-CLI(1)

## NAME
**tizen-thumbnail-util-cli** - Extract thumbnails from media files using
the public Tizen thumbnail utility API.

## SYNOPSIS
**tizen-thumbnail-util-cli** *COMMAND* [*OPTIONS*]  
**tizen-thumbnail-util-cli** **-h** | **--help** | **help**  
**tizen-thumbnail-util-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-thumbnail-util-cli** provides command-line access to the public
thumbnail extraction APIs from `thumbnail_util.h`.

The tool supports:
- extracting a thumbnail directly to an output file
- extracting a thumbnail to memory and printing generated buffer
  metadata
- validating requested dimensions before calling the native API

The requested width and height must be between `1` and `2000`. The
generated thumbnail may be smaller than the requested size because the
native API preserves aspect ratio and may clamp oversized requests to
the original media resolution.

If the input or output path points to media or external storage, the
underlying API may require
`http://tizen.org/privilege/mediastorage` or
`http://tizen.org/privilege/externalstorage`.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **extract-file** | `<SRC> <DST> <WIDTH> <HEIGHT>` | Storage dependent | Calls `thumbnail_util_extract_to_file()` and writes the generated thumbnail to `<DST>`. |
| **extract-buffer-info** | `<SRC> <WIDTH> <HEIGHT>` | Storage dependent | Calls `thumbnail_util_extract_to_buffer()` and prints generated width, height, and buffer size. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or thumbnail API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-thumbnail-util-cli`

## AUTHOR
Part of the **Tizen Tools** project.

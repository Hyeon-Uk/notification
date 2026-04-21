# TIZEN-METADATA-EXTRACTOR-CLI(1)

## NAME
**tizen-metadata-extractor-cli** - Extract metadata, artwork, and video
frames from media files.

## SYNOPSIS
**tizen-metadata-extractor-cli** *COMMAND* [*OPTIONS*]  
**tizen-metadata-extractor-cli** **-h** | **--help** | **help**  
**tizen-metadata-extractor-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-metadata-extractor-cli** provides command-line access to the
public Tizen Metadata Extractor APIs from `metadata_extractor.h` and
`metadata_extractor_type.h`.

The tool supports:
- reading public metadata attributes such as duration, codec, title, and
  stream properties
- reading embedded artwork metadata through
  `metadata_extractor_get_artwork()`
- reading raw RGB888 video frame payload sizes through
  `metadata_extractor_get_frame()` and
  `metadata_extractor_get_frame_at_time()`

When reading files in media or external storage, the caller may need
`http://tizen.org/privilege/mediastorage` or
`http://tizen.org/privilege/externalstorage`.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **extract** | `<file>` | Opens the file with `metadata_extractor_set_path()` and prints every non-empty public metadata attribute supported by this CLI. |
| **get** | `<file> <attribute>` | Reads one public metadata attribute using `metadata_extractor_get_metadata()`. |
| **artwork-info** | `<file>` | Reads embedded artwork size and MIME type using `metadata_extractor_get_artwork()`. |
| **frame-info** | `<file>` | Reads the default RGB888 video frame size using `metadata_extractor_get_frame()`. |
| **frame-at-time** | `<file> <timestamp-ms> [accurate\|nearest]` | Reads a frame near the requested timestamp using `metadata_extractor_get_frame_at_time()`. |
| **list-attrs** | None | Lists the public metadata attribute keys supported by `get` and `extract`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, missing file access, or Metadata Extractor API
failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-metadata-extractor-cli`

## AUTHOR
Part of the **Tizen Tools** project.

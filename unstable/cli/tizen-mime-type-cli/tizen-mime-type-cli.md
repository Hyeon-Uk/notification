# TIZEN-MIME-TYPE-CLI(1)

## NAME
**tizen-mime-type-cli** - Resolve MIME types and file extensions through
the public Tizen MIME Type API.

## SYNOPSIS
**tizen-mime-type-cli** *COMMAND* [*OPTIONS*]  
**tizen-mime-type-cli** **-h** | **--help** | **help**  
**tizen-mime-type-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-mime-type-cli** provides command-line access to the public MIME
Type APIs from `mime_type.h`.

The tool supports:
- resolving a MIME type from a file extension with
  `mime_type_get_mime_type()`
- resolving registered file extensions from a MIME type with
  `mime_type_get_file_extension()`
- detecting a MIME type from an on-device file path with
  `mime_type_get_mime_type_for_file()`
- detecting a MIME type from sampled file bytes with
  `mime_type_get_mime_type_for_data()`

When reading files in media or external storage, the caller may need
`http://tizen.org/privilege/mediastorage` or
`http://tizen.org/privilege/externalstorage`.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **get-mime** | `<extension>` | Resolves the MIME type for the given file extension. Unknown extensions resolve to `application/octet-stream` through the underlying API. |
| **get-ext** | `<mime-type>` | Resolves the registered file extensions for the given MIME type and prints the returned list length. |
| **from-file** | `<path>` | Detects a MIME type from the file path using `mime_type_get_mime_type_for_file()`. |
| **from-data** | `<path>` | Reads up to 512 bytes from the file and detects a MIME type using `mime_type_get_mime_type_for_data()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, file access failure, or MIME Type API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-mime-type-cli`

## AUTHOR
Part of the **Tizen Tools** project.

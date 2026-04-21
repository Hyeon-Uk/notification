# TIZEN-MEDIATOOL-CLI(1)

## NAME
**tizen-mediatool-cli** - Create and inspect Tizen media format handles.

## SYNOPSIS
**tizen-mediatool-cli** *COMMAND* [*OPTIONS*]  
**tizen-mediatool-cli** **-h** | **--help** | **help**  
**tizen-mediatool-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-mediatool-cli** provides command-line access to the public
Tizen Media Tool format APIs from `media_format.h`.

The tool supports:
- creating audio and video `media_format_h` values and reading them back
- checking the public format type reported by `media_format_get_type()`
- exercising `media_format_ref()` and `media_format_make_writable()`
- reading container, text, and AAC header metadata through the matching
  getter APIs

The commands operate on short-lived in-process format handles only. They
do not require media playback, files, or device privileges.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **create-audio** | `<mime> <channels> <rate> <bit>` | Creates an audio format, applies the public audio setter APIs, and prints the values returned by `media_format_get_audio_info()`. |
| **create-video** | `<mime> <width> <height> <fps>` | Creates a video format, applies the public video setter APIs, and prints the values returned by `media_format_get_video_info()` and `media_format_get_video_frame_rate()`. |
| **get-type** | `<audio\|video\|text\|container>` | Creates a format of the requested kind and prints the public type value returned by `media_format_get_type()`. |
| **make-writable** | `<audio\|video>` | Increases the reference count, calls `media_format_make_writable()`, and reports whether the API returned the original handle or a copied writable handle. |
| **container-info** | `<mp4\|avi\|webm\|wav>` | Sets a public container MIME and reads it back using `media_format_get_container_mime()`. |
| **text-info** | `<mp4\|3gp> <none\|timed-text-mp4\|timed-text-3gpp>` | Sets public text MIME and text type values, then reads them back with `media_format_get_text_info()`. |
| **aac-info** | `<aac\|aac-lc\|aac-he> <none\|adts\|adif>` | Sets the AAC header type and reads back both `media_format_get_audio_aac_header_type()` and `media_format_get_audio_aac_type()`. |
| **verify** | None | Creates a format handle and prints the writability reported by `media_format_is_writable()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, unsupported command input, or Media Tool API
failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-mediatool-cli`

## AUTHOR
Part of the **Tizen Tools** project.

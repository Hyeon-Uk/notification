# TIZEN-MEDIACODEC-CLI(1)

## NAME
**tizen-mediacodec-cli** - inspect supported media codecs and configure
basic decoder or encoder parameters through the Tizen MediaCodec API

## SYNOPSIS
**tizen-mediacodec-cli** **list-codecs**

**tizen-mediacodec-cli** **check** *CODEC* *dec|enc*

**tizen-mediacodec-cli** **setup-vdec** *CODEC* *WIDTH* *HEIGHT*

**tizen-mediacodec-cli** **setup-venc** *CODEC* *WIDTH* *HEIGHT* *FPS*
*BITRATE*

**tizen-mediacodec-cli** **setup-adec** *CODEC* *RATE* *CHANNELS* *BIT*

**tizen-mediacodec-cli** **setup-aenc** *CODEC* *RATE* *CHANNELS* *BIT*
*BITRATE*

**tizen-mediacodec-cli** **help**

**tizen-mediacodec-cli** **version**

## DESCRIPTION
**tizen-mediacodec-cli** provides a shell-facing wrapper around the
public `capi-media-codec` APIs used to enumerate supported codecs,
inspect decoder or encoder support, and validate basic audio or video
configuration calls.

The tool stays within synchronous public APIs that are safe to exercise
from the shell. When the current emulator or device cannot provide the
required runtime service, the command reports a documented `[WARN]`
fallback instead of claiming success for unsupported behavior.

## COMMANDS REFERENCE

| Command | Description |
| :--- | :--- |
| **list-codecs** | Enumerate codec types reported by `mediacodec_foreach_supported_codec()`. |
| **check** *CODEC* *dec\|enc* | Query hardware and software support for a codec with `mediacodec_get_supported_type()`. |
| **setup-vdec** *CODEC* *WIDTH* *HEIGHT* | Create a codec handle, select decoder mode, and apply `mediacodec_set_vdec_info()`. |
| **setup-venc** *CODEC* *WIDTH* *HEIGHT* *FPS* *BITRATE* | Create a codec handle, select encoder mode, and apply `mediacodec_set_venc_info()`. |
| **setup-adec** *CODEC* *RATE* *CHANNELS* *BIT* | Create a codec handle, select decoder mode, and apply `mediacodec_set_adec_info()`. |
| **setup-aenc** *CODEC* *RATE* *CHANNELS* *BIT* *BITRATE* | Create a codec handle, select encoder mode, and apply `mediacodec_set_aenc_info()`. |
| **help** | Show the built-in usage summary. |
| **version** | Print the CLI version string. |

Supported codec names follow the aliases implemented in the CLI, such as
`h264`, `hevc`, `aac`, `aac-he`, `mp3`, `vorbis`, `flac`, and `opus`.

## EXIT STATUS
**0**: The command completed successfully or reported a documented
runtime `[WARN]` fallback.

**1**: Invalid arguments or an unsupported codec name were provided.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-mediacodec-cli`

## AUTHOR
Part of the **Tizen Tools** project.

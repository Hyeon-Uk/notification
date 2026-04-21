# TIZEN-IMAGE-UTIL-CLI(1)

## NAME
**tizen-image-util-cli** - Decode, inspect, transform, and encode images.

## SYNOPSIS
**tizen-image-util-cli** *COMMAND* [*ARGS*]  
**tizen-image-util-cli** **-h** | **--help** | **help**  
**tizen-image-util-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-image-util-cli** provides one-shot wrappers over the non-deprecated
Image Util APIs from `image_util.h`, `image_util_decode.h`, and
`image_util_encode.h`.

The tool supports representative-color extraction from raw RGB data,
colorspace capability inspection, image metadata inspection, synchronous image
decode/transform/encode flows, JPEG downscale decoding, and in-memory buffer
encoding.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **extract-color** | `<width> <height> <raw_rgb888_file>` | Extracts a representative RGB color from raw RGB888 bytes. |
| **supported-colorspace** | `<format>` | Lists the colorspaces supported for the selected image format. |
| **inspect** | `<src_image_file> [colorspace]` | Decodes an image and prints width, height, colorspace, and decoded buffer size. |
| **transform** | `<width> <height> <rotation> <src> <dst_format> <dst_file>` | Decodes, resizes, rotates/flips, and encodes an image to a file. |
| **decode** | `<colorspace> <src_image_file> <dst_raw_file>` | Decodes an image to raw bytes in the requested colorspace. |
| **decode-scale** | `<colorspace> <1\|2\|4\|8> <src_image_file> <dst_raw_file>` | Decodes a JPEG image with the requested downscale factor. |
| **encode** | `<format> <colorspace> <width> <height> <src_raw_file> <dst_image_file>` | Encodes raw image bytes to an output file. |
| **encode-to-buffer** | `<format> <colorspace> <width> <height> <src_raw_file>` | Encodes raw image bytes to memory and prints the encoded size. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, decode/encode failure, unsupported format, or file I/O
failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-image-util-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-CAPI-MEDIA-VISION-DL-CLI(1)

## NAME
**tizen-capi-media-vision-dl-cli** - Inspect Media Vision sources and run
non-deprecated DL inference tasks.

## SYNOPSIS
**tizen-capi-media-vision-dl-cli** *COMMAND* [*OPTIONS*]  
**tizen-capi-media-vision-dl-cli** **-h** | **--help** | **help**  
**tizen-capi-media-vision-dl-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-capi-media-vision-dl-cli** provides command-line access to the
current public Tizen Media Vision image-classification and
object-detection APIs from `mv_common.h`,
`mv_image_classification.h`, and `mv_object_detection.h`.

The tool decodes an input image through `image_util`, fills a
`mv_source_h` buffer, and then either:
- prints source metadata
- performs image classification
- performs object detection

The tool intentionally excludes deprecated Media Vision families such as
`mv_inference_*`, face detection, and face recognition.

## COMMANDS REFERENCE

| Command | Args | API Scope | Description |
| :--- | :--- | :---: | :--- |
| **source-info** | `--image <PATH>` | Standard | Loads the image into `mv_source_h` and prints width, height, buffer size, and colorspace. |
| **classify** | `--image <PATH>` | Standard | Runs `mv_image_classification_*` inference and prints the returned labels for the processed frame. |
| **detect-object** | `--image <PATH>` | Standard | Runs `mv_object_detection_*` inference and prints detected bounding boxes for the processed frame. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, image decode/load failure, inference setup
failure, or inference API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-capi-media-vision-dl-cli`

## AUTHOR
Part of the **Tizen Tools** project.

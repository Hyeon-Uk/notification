---
name: tizen-image-vision
description: Provides image processing, computer vision, deep learning inference, and thumbnail generation capabilities. Use when processing images, generating thumbnails, or performing computer vision operations.
---

# Tizen Image Vision

Provides image processing, computer vision, and deep learning capabilities.

## Quick reference

**Image Util**: Image processing → `tizen-image-util-cli`
**Thumbnail Util**: Thumbnail generation → `tizen-thumbnail-util-cli`
**Media Vision DL**: Deep learning → `tizen-capi-media-vision-dl-cli`
**EFL Util**: EFL utilities → `tizen-efl-util-cli`

## Image Util

Process and manipulate images:

```bash
# Get image info
tizen-image-util-cli info --file image.jpg

# Resize image
tizen-image-util-cli resize --input image.jpg --output resized.jpg --width 800 --height 600

# Rotate image
tizen-image-util-cli rotate --input image.jpg --output rotated.jpg --angle 90

# Crop image
tizen-image-util-cli crop --input image.jpg --output cropped.jpg --x 0 --y 0 --width 400 --height 400
```

See [tizen-image-util-cli.md](references/tizen-image-util-cli.md) for complete documentation.

## Thumbnail Util

Generate thumbnails for media files:

```bash
# Generate thumbnail
tizen-thumbnail-util-cli extract --file video.mp4 --output thumb.jpg

# Extract at specific time
tizen-thumbnail-util-cli extract --file video.mp4 --output thumb.jpg --time 5.0

# Get thumbnail info
tizen-thumbnail-util-cli info --file thumb.jpg
```

See [tizen-thumbnail-util-cli.md](references/tizen-thumbnail-util-cli.md) for complete documentation.

## Media Vision DL

Execute deep learning inference:

```bash
# Get model info
tizen-capi-media-vision-dl-cli model-info --path /path/to/model.tflite

# Run inference
tizen-capi-media-vision-dl-cli infer --model /path/to/model.tflite --input /path/to/image.jpg

# Get supported models
tizen-capi-media-vision-dl-cli list-models
```

See [tizen-capi-media-vision-dl-cli.md](references/tizen-capi-media-vision-dl-cli.md) for complete documentation.

## EFL Util

EFL utility functions:

```bash
# Get EFL version
tizen-efl-util-cli version

# List EFL components
tizen-efl-util-cli list-components
```

See [tizen-efl-util-cli.md](references/tizen-efl-util-cli.md) for complete documentation.

## Common use cases

- **Photo editing**: Process and manipulate images
- **Gallery apps**: Generate thumbnails for media files
- **Computer vision**: Run AI-powered image analysis
- **Deep learning**: Execute inference with ML models

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
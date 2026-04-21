---
name: tizen-media-utilities
description: Provides media utility functions including audio I/O, media codec operations, metadata extraction, and media tool operations. Use when processing media files, extracting metadata, or working with media codecs.
---

# Tizen Media Utilities

Provides media utility functions for processing, codec operations, and metadata extraction.

## Quick reference

**Audio I/O**: Audio input/output → `tizen-audio-io-cli`
**Media Codec**: Codec operations → `tizen-mediacodec-cli`
**Metadata Extractor**: Extract metadata → `tizen-metadata-extractor-cli`
**Media Tool**: Media utilities → `tizen-mediatool-cli`

## Audio I/O

Manage audio input/output devices:

```bash
# List audio devices
tizen-audio-io-cli list-devices

# Get audio device info
tizen-audio-io-cli device-info --id 0

# Set audio device
tizen-audio-io-cli set-device --id 1
```

See [tizen-audio-io-cli.md](references/tizen-audio-io-cli.md) for complete documentation.

## Media Codec

Query and manage media codecs:

```bash
# List supported codecs
tizen-mediacodec-cli list-codecs

# Check codec support
tizen-mediacodec-cli check h264 dec

# Setup video decoder
tizen-mediacodec-cli setup-vdec h264 1920 1080
```

See [tizen-mediacodec-cli.md](references/tizen-mediacodec-cli.md) for complete documentation.

## Metadata Extractor

Extract metadata from media files:

```bash
# Extract all metadata
tizen-metadata-extractor-cli extract --file /opt/media/video.mp4

# Get specific metadata
tizen-metadata-extractor-cli get --file video.mp4 --key duration

# Extract thumbnail
tizen-metadata-extractor-cli thumbnail --file video.mp4 --output thumb.jpg
```

See [tizen-metadata-extractor-cli.md](references/tizen-metadata-extractor-cli.md) for complete documentation.

## Media Tool

General media utility operations:

```bash
# Get media info
tizen-mediatool-cli info --file video.mp4

# Convert media format
tizen-mediatool-cli convert --input video.mp4 --output video.avi
```

See [tizen-mediatool-cli.md](references/tizen-mediatool-cli.md) for complete documentation.

## Common use cases

- **Media processing**: Convert and process media files
- **Metadata queries**: Extract file metadata for cataloging
- **Codec management**: Check codec support and availability
- **Audio management**: Control audio input/output devices

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
---
name: tizen-media-player
description: Controls media playback operations including audio/video playback, media content queries, player display management, and media key control. Use when playing media files, managing playback state, or querying media databases.
---

# Tizen Media Player

Controls media playback, queries media content, and manages player display and media keys.

## Quick reference

**Media Content**: Query media database → `tizen-media-content-cli`
**Media Player**: Playback control → `tizen-player-cli`
**Player Display**: Display management → `tizen-player-display-cli`
**Play Content**: Content playback → `tizen-play-content-cli`
**Media Key**: Key control → `tizen-media-key-cli`

## Media Content

Query and browse media content database:

```bash
# List all media items
tizen-media-content-cli scan --path /opt/usr/media

# Get media metadata
tizen-media-content-cli get --uri file:///opt/media/video.mp4

# Search media by type
tizen-media-content-cli search --type video
```

See [tizen-media-content-cli.md](references/tizen-media-content-cli.md) for complete documentation.

## Media Player

Control media playback state:

```bash
# Start playback
tizen-player-cli play --uri file:///opt/media/audio.mp3

# Control playback
tizen-player-cli pause
tizen-player-cli resume
tizen-player-cli stop

# Seek in media
tizen-player-cli seek --position 30000
```

See [tizen-player-cli.md](references/tizen-player-cli.md) for complete documentation.

## Player Display

Manage player display properties:

```bash
# Set display mode
tizen-player-display-cli set-get-mode --mode 2

# Set rotation
tizen-player-display-cli set-get-rotation --rotation 90

# Set visibility
tizen-player-display-cli set-get-visible --visible true

# Get video size
tizen-player-display-cli get-video-size
```

See [tizen-player-display-cli.md](references/tizen-player-display-cli.md) for complete documentation.

## Play Content

Launch content with default player:

```bash
# Play video file
tizen-play-content-cli play --uri file:///opt/media/video.mp4

# Play with app control
tizen-play-content-cli play --operation view --uri file:///opt/media/image.jpg
```

See [tizen-play-content-cli.md](references/tizen-play-content-cli.md) for complete documentation.

## Media Key

Control media playback via media keys:

```bash
# Reserve media key
tizen-player-display-cli reserve --timeout 5.0

# Release media key
tizen-media-key-cli release

# List available keys
tizen-media-key-cli list-keys

# List key statuses
tizen-media-key-cli list-statuses
```

See [tizen-media-key-cli.md](references/tizen-media-key-cli.md) for complete documentation.

## Common use cases

- **Media player apps**: Play audio and video files
- **Gallery apps**: Display and browse media content
- **Video streaming**: Control playback state and display
- **Media center**: Integrate with media key controls

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
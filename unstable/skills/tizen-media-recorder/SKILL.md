---
name: tizen-media-recorder
description: Manages audio and video recording operations including sound management, sound pool, and WAV playback. Use when recording audio/video, managing audio devices, or controlling sound output.
---

# Tizen Media Recorder

Manages audio/video recording, sound management, and sound pool operations.

## Quick reference

**Recorder**: Audio/video recording → `tizen-recorder-cli`
**Sound Manager**: Sound control → `tizen-sound-manager-cli`
**Sound Pool**: Sound pool management → `tizen-sound-pool-cli`
**WAV Player**: WAV playback → `tizen-wav-player-cli`

## Recorder

Record audio and video:

```bash
# Check supported codecs
tizen-recorder-cli supported-codecs

# Check supported formats
tizen-recorder-cli supported-formats

# Setup recording
tizen-recorder-cli setup aac mp4 /tmp/record.mp4

# Start recording
tizen-recorder-cli start

# Stop recording
tizen-recorder-cli stop
```

See [tizen-recorder-cli.md](references/tizen-recorder-cli.md) for complete documentation.

## Sound Manager

Manage sound devices and volume:

```bash
# Get current sound type
tizen-sound-manager-cli current-type

# List all sound types
tizen-sound-manager-cli list-types

# Get volume
tizen-sound-manager-cli get-volume --type media

# Set volume
tizen-sound-manager-cli set-volume --type voice --value 5

# List output devices
tizen-sound-manager-cli list-devices --direction output
```

See [tizen-sound-manager-cli.md](references/tizen-sound-manager-cli.md) for complete documentation.

## Sound Pool

Manage sound pool for audio effects:

```bash
# Load sound to pool
tizen-sound-pool-cli load --id 1 --path /opt/media/effect.wav

# Play sound
tizen-sound-pool-cli play --id 1

# Unload sound
tizen-sound-pool-cli unload --id 1
```

See [tizen-sound-pool-cli.md](references/tizen-sound-pool-cli.md) for complete documentation.

## WAV Player

Play WAV audio files:

```bash
# Play WAV file
tizen-wav-player-cli play --file /opt/media/audio.wav

# Set volume
tizen-wav-player-cli set-volume --level 0.8
```

See [tizen-wav-player-cli.md](references/tizen-wav-player-cli.md) for complete documentation.

## Common use cases

- **Voice recording**: Record audio notes and memos
- **Video recording**: Capture video content
- **Audio playback**: Play sound effects and music
- **Sound management**: Control volume and audio devices

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
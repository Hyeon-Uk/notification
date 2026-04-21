---
name: tizen-voice-speech
description: Provides speech-to-text (STT), text-to-speech (TTS), and voice control operations. Use when converting speech to text, text to speech, or controlling applications via voice commands.
---

# Tizen Voice Speech

Provides speech-to-text, text-to-speech, and voice control operations.

## Quick reference

**STT**: Speech-to-Text → `tizen-stt-cli`
**TTS**: Text-to-Speech → `tizen-tts-cli`
**Voice Control**: Voice commands → `tizen-voice-control-cli`

## STT (Speech-to-Text)

Convert speech to text:

```bash
# Get supported languages
tizen-stt-cli get-supported-languages

# Set language
tizen-stt-cli set-language en_US

# Start recognition
tizen-stt-cli start

# Stop recognition
tizen-stt-cli stop
```

See [tizen-stt-cli.md](references/tizen-stt-cli.md) for complete documentation.

## TTS (Text-to-Speech)

Convert text to speech:

```bash
# Get supported voices
tizen-tts-cli get-supported-voices

# Set voice
tizen-tts-cli set-voice en_US

# Speak text
tizen-tts-cli speak --text "Hello World"

# Get state
tizen-tts-cli get-state
```

See [tizen-tts-cli.md](references/tizen-tts-cli.md) for complete documentation.

## Voice Control

Control applications via voice commands:

```bash
# Get languages
tizen-voice-control-cli languages

# Get current language
tizen-voice-control-cli current-language

# Get states
tizen-voice-control-cli states

# Set invocation name
tizen-voice-control-cli set-invocation-name bixby

# Prepare roundtrip
tizen-voice-control-cli prepare-roundtrip

# Request dialog
tizen-voice-control-cli request-dialog hello world true
```

See [tizen-voice-control-cli.md](references/tizen-voice-control-cli.md) for complete documentation.

## Common use cases

- **Voice assistants**: Convert speech to text and vice versa
- **Accessibility**: Provide text-to-speech for visually impaired
- **Voice control**: Control applications via voice commands

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
# Tizen CLI Tools Index

This document provides an overview of all available CLI tools in the tizen-tools package. These tools are designed for direct system interaction and can be used by AI agents for various Tizen embedded system operations.

## Available CLI Tools

| Tool Name | Description | Category | Documentation |
|-----------|-------------|----------|---------------|
| tizen-app-manager-cli | Manage applications: list, terminate, launch via app_control, query package info | App Management | [tool.md](cli/tizen-app-manager-cli/tool.md) |
| tizen-aurum-cli | UI automation via Aurum gRPC: send remote keys, click elements, inspect accessibility tree, navigate focus | UI Control | [tool.md](cli/tizen-aurum-cli/tool.md) |
| tizen-context-cli | Query app/broadcast context, screenshots, and key events from agent-context-service via gRPC | Context | [tool.md](cli/tizen-context-cli/tool.md) |
| tizen-control-display-cli | Native CLI tool to manage display brightness | Hardware Control | [tool.md](cli/tizen-control-display-cli/tool.md) |
| tizen-device-info-cli | Query device hardware and system information | Device Info | [tool.md](cli/tizen-device-info-cli/tool.md) |
| tizen-file-manager-cli | File system operations — read, write, append, copy, move, remove, list, stat, mkdir | File Management | [tool.md](cli/tizen-file-manager-cli/tool.md) |
| tizen-hardware-control-cli | Control device hardware: haptic vibration, LED flash, power locks, feedback patterns | Hardware Control | [tool.md](cli/tizen-hardware-control-cli/tool.md) |
| tizen-hud-cli | Control the agent HUD overlay on Tizen TV: show/hide text overlays and content recommendations | UI Control | [tool.md](cli/tizen-hud-cli/tool.md) |
| tizen-network-info-cli | Query network, Wi-Fi, Bluetooth status, and data usage | Network | [tool.md](cli/tizen-network-info-cli/tool.md) |
| tizen-notification-cli | Send notifications and schedule alarms | Notifications | [tool.md](cli/tizen-notification-cli/tool.md) |
| tizen-play-content-cli | Play content on Tizen TV by provider and content ID using wascmd deeplinks | Content | [tool.md](cli/tizen-play-content-cli/tool.md) |
| tizen-search-content-cli | Search movies, TV shows, and YouTube videos using Samsung SearchAll API or YouTube web scraping | Content | [tool.md](cli/tizen-search-content-cli/tool.md) |
| tizen-sensor-cli | Read sensor data from device sensors | Sensors | [tool.md](cli/tizen-sensor-cli/tool.md) |
| tizen-sound-cli | Control volume levels, list sound devices, and play tones | Audio | [tool.md](cli/tizen-sound-cli/tool.md) |
| tizen-web-search-cli | Multi-engine web search supporting Naver, Google, Brave, Gemini, Grok, Kimi, and Perplexity | Web Search | [tool.md](cli/tizen-web-search-cli/tool.md) |
| tizen-presentation-validate-cli | Validate presentation JSON files against the Samsung Tizen TV viewer schema | Content | [tool.md](cli/tizen-presentation-validate-cli/tool.md) |
| tizen-viewer-launch-cli | Launch org.tizen.tizen-tool-viewer with presentation JSON via Tizen App Control | Content | [tool.md](cli/tizen-viewer-launch-cli/tool.md) |

## Usage Pattern

All CLI tools follow a consistent pattern:
```bash
tizen-{tool-name}-cli [subcommand] [options]
```

Most tools output JSON format for easy parsing by AI agents.

## Quick Reference by Category

### App Management
- `tizen-app-manager-cli` - Application lifecycle management

### Device Information
- `tizen-device-info-cli` - Comprehensive device information
- `tizen-sensor-cli` - Sensor data reading
- `tizen-network-info-cli` - Network connectivity status

### Hardware Control
- `tizen-control-display-cli` - Display brightness control
- `tizen-hardware-control-cli` - Haptic, LED, power management

### Content & Media
- `tizen-search-content-cli` - Content discovery
- `tizen-play-content-cli` - Content playback
- `tizen-sound-cli` - Audio control
- `tizen-presentation-validate-cli` - Presentation JSON validation
- `tizen-viewer-launch-cli` - Viewer launch with presentation JSON

### File System
- `tizen-file-manager-cli` - File operations

### UI & Context
- `tizen-aurum-cli` - UI automation: keys, clicks, accessibility tree, focus navigation
- `tizen-context-cli` - App context and screenshots
- `tizen-hud-cli` - HUD overlay control

### Notifications
- `tizen-notification-cli` - System notifications

### Web Services
- `tizen-web-search-cli` - Web search capabilities

For detailed usage of each tool, refer to the individual tool documentation linked in the table above.
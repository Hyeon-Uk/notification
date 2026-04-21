# Tizen Tools - Main Reference Guide

This is the main reference guide for all tools and skills available in the tizen-tools package. This document should be the first point of reference for AI agents working with Tizen embedded systems.

## Overview

The tizen-tools package provides three main categories of tools:

1. **CLI Tools** - Direct command-line interface tools for specific system operations
2. **Skills** - High-level orchestrated workflows combining multiple CLI tools
3. **System CLI** - Documentation for Tizen system command-line interface tools

## Quick Start

For immediate access to tool documentation:
- **CLI Tools Index**: See [cli/index.md](cli/index.md)
- **Skills Index**: See [skills/index.md](skills/index.md)
- **System Commands**: See [system_cli/index.md](system_cli/index.md)

## Directory Structure

```
/opt/usr/share/tizen-tools/tools/
├── tools.md              # This main reference guide
├── cli/                  # CLI Tools directory
│   ├── index.md         # CLI tools index
│   ├── tizen-app-manager-cli/
│   ├── tizen-context-cli/
│   └── ... (15 total tools)
├── skills/               # Skills directory
│   ├── index.md         # Skills index
│   ├── agent-browser/
│   ├── tizen-content/
│   ├── tizen-context/
│   ├── tizen-hud/
│   ├── tizen-ui-navigation/
│   ├── smartthings/
│   └── smartthings-curl/
└── system_cli/           # System CLI documentation directory
    ├── index.md         # System CLI index
    ├── accessibility/
    ├── action_framework/
    ├── application/
    └── ... (26 categories total)
```

## Tool Categories

### 1. CLI Tools ([cli/index.md](cli/index.md))

Individual command-line tools for direct system interaction:

```
tizen-{tool-name}-cli [subcommand] [options]
```

**Available CLI Tools (15 tools):**

| Tool | Description |
|------|-------------|
| `tizen-app-manager-cli` | Application management: list, terminate, launch via app_control, query package info |
| `tizen-aurum-cli` | UI automation via Aurum gRPC: send remote keys, click elements, inspect accessibility tree, navigate focus |
| `tizen-context-cli` | Query app/broadcast context, screenshots, and key events from agent-context-service via gRPC |
| `tizen-control-display-cli` | Native CLI tool to manage display brightness |
| `tizen-device-info-cli` | Query device hardware and system information |
| `tizen-file-manager-cli` | File system operations — read, write, append, copy, move, remove, list, stat, mkdir |
| `tizen-hardware-control-cli` | Control device hardware: haptic vibration, LED flash, power locks, feedback patterns |
| `tizen-hud-cli` | Control the agent HUD overlay on Tizen TV: show/hide text overlays and content recommendations |
| `tizen-network-info-cli` | Query network, Wi-Fi, Bluetooth status, and data usage |
| `tizen-notification-cli` | Send notifications and schedule alarms |
| `tizen-play-content-cli` | Play content on Tizen TV by provider and content ID using wascmd deeplinks |
| `tizen-search-content-cli` | Search movies, TV shows, and YouTube videos using Samsung SearchAll API or YouTube web scraping |
| `tizen-sensor-cli` | Read sensor data from device sensors |
| `tizen-sound-cli` | Control volume levels, list sound devices, and play tones |
| `tizen-web-search-cli` | Multi-engine web search supporting Naver, Google, Brave, Gemini, Grok, Kimi, and Perplexity |

### 2. Skills ([skills/index.md](skills/index.md))

Orchestrated workflows combining multiple CLI tools:

**Available Skills:**

| Skill | Description |
|-------|-------------|
| `tizen-content` | Search and play content on Samsung TV with recommendation UI |
| `tizen-context` | Queries app/broadcast/HUD context, captures screenshots, and gets key event history |
| `tizen-hud` | Controls the agent-hud-service overlay and recommendation UI on Tizen TV |
| `tizen-ui-navigation` | Navigates Tizen TV apps via remote keys and accessibility tree, with per-app guides for CSFS, YouTube, and Samsung TV Plus |
| `agent-browser` | Browser automation CLI for AI agents — navigate, fill forms, click, screenshot, scrape, and authenticate via the `agent-browser` CLI |
| `smartthings` | SmartThings device control and automation |
| `smartthings-curl` | SmartThings API access via curl commands |

### 3. System CLI ([system_cli/index.md](system_cli/index.md))

Documentation for Tizen system command-line interface tools organized by category:

**Categories (26 categories, 150+ tools):**

| Category | Description | Tools |
|----------|-------------|-------|
| `accessibility/` | Accessibility-related CLI tools | at_spi2_tool |
| `action_framework/` | Action Framework, alarms, preferences, themes | actdctl, alarmmgr_tool, preference_tool, theme-cmd, etc. |
| `application/` | Application lifecycle and management | app_launcher, appmgr_tool, aulctl, launch_app, etc. |
| `audio/` | ALSA and MIDI audio tools | aplay, arecord, amixer, alsactl, etc. |
| `compression/` | File compression and archive tools | zip, unzip, zipinfo, etc. |
| `configuration/` | System configuration and library management | ldconfig, ldd, vconftool, update-alternatives, etc. |
| `database/` | Database management tools | sqlite3, buxton2ctl |
| `dbus/` | D-Bus inter-process communication tools | dbuspolicy-checker |
| `device/` | Device management and hardware abstraction | devicectl, lshal, etc. |
| `device_mapper/` | Device mapper administration tools | dmsetup, dmstats, blkdeactivate |
| `display/` | Display and window management tools | winfo |
| `dotnet/` | .NET runtime tools | dotnettool |
| `filesystem/` | Filesystem management (btrfs, f2fs, etc.) | btrfs, mkfs.f2fs, fsck.f2fs, tar, etc. |
| `gstreamer/` | GStreamer multimedia framework tools | gst-launch-1.0, gst-inspect-1.0, etc. |
| `json/` | JSON processing tools | json-glib-format, json-glib-validate |
| `kernel_module/` | Linux kernel module management tools | modprobe, insmod, rmmod, lsmod, etc. |
| `locale/` | Locale, i18n, character encoding tools | locale, iconv, tzselect, zic, etc. |
| `logging/` | System logging and DLog tools | dlogutil, dlogctl, dlogsend, etc. |
| `network/` | Network configuration and management | connmanctl, iptables, wifi_connect_tool, etc. |
| `package_management/` | Tizen package management tools | pkgcmd, pkginfo, pkg-db-recovery, etc. |
| `resource/` | Resource management tools | res-copy, rsc-slice |
| `security/` | Security, SMACK, and encryption tools | chsmack, smackctl, cryptsetup, ckm_tool, etc. |
| `service/` | System service and daemon management tools | compmgr_tool, resource-monitor, etc. |
| `system/` | System monitoring and control tools | ps, free, uptime, vmstat, sysctl, etc. |
| `text_processing/` | Text processing and transformation tools | awk, sed, xsltproc, etc. |
| `tizen_platform/` | Tizen platform-specific utilities | tzplatform-get, tlm-launcher, system-info-tool, etc. |

## Usage Patterns

### JSON Output Standard
Most tools output structured JSON for easy parsing:
```json
{
  "status": "success|error",
  "tool": "tool-name",
  "message": "Human-readable message",
  "data": { ... }
}
```

### Pipe Chaining
Tools support stdin/stdout piping for workflow creation:
```bash
tool1 | tool2 | tool3
```

### Error Handling
- Exit code `0`: Success
- Exit code `1`: Runtime error
- Exit code `2`: Usage error
- JSON error format for programmatic handling

## Common Workflows

### Content Discovery and Playback
```bash
# Search content
tizen-search-content-cli "Batman" --limit 3

# Play directly
tizen-search-content-cli "Batman" --youtube --limit 1 | tizen-play-content-cli

# Show recommendations
tizen-search-content-cli "Batman" --limit 5 | tizen-hud-cli show-reco --expand
```

### System Information Gathering
```bash
# Get comprehensive system info
tizen-device-info-cli system-info

# Check battery status
tizen-device-info-cli battery

# Get runtime stats
tizen-device-info-cli runtime
```

### Context Awareness
```bash
# Get current app context
tizen-context-cli context

# Analyze recent screen content
tizen-context-cli screenshot --list --window 10s

# Get key event history
tizen-context-cli keys --count 20
```

### Hardware Control
```bash
# Control display brightness
tizen-control-display-cli --brightness 75

# Vibrate haptic motor
tizen-hardware-control-cli haptic --duration 1000

# Flash LED
tizen-hardware-control-cli led --action on --brightness 50
```

## Decision Making Guide

### When to Use CLI Tools
- Direct, specific system interactions needed
- Simple, atomic operations
- Maximum performance required
- Testing/debugging specific functionality
- When you know exactly what action to take

### When to Use Skills
- Complex workflows requiring multiple tools
- User intent interpretation needed
- Error handling and fallback logic required
- UI interaction and state management
- Content discovery and recommendation scenarios

### When to Use System CLI Documentation
- Looking for Tizen system command documentation
- Need to understand system tool capabilities
- Exploring available system utilities
- Finding appropriate tools for system-level operations

### Service Dependencies
Some tools require specific services to be running:
- `tizen-aurum-cli`: Requires `org.tizen.aurum-bootstrap` (auto-launched via app_control)
- `tizen-context-cli`: Requires `agent-context-service`
- `tizen-hud-cli`: Requires `agent-hud-service`

## Best Practices

### 1. Start with Context
Before taking action, understand the current system state:
```bash
tizen-context-cli context
```

### 2. Use Appropriate Error Handling
Check exit codes and parse JSON error responses:
```bash
if ! tizen-device-info-cli battery; then
  echo "Failed to get battery info"
fi
```

### 3. Leverage Pipe Chaining
Combine tools for efficient workflows:
```bash
# Build rich recommendation sets
tizen-search-content-cli "review" --youtube --limit 2 \
  | tizen-search-content-cli "Batman" --limit 3 \
  | tizen-hud-cli show-reco --expand
```

### 4. Provide User Feedback
Use HUD overlay for status updates:
```bash
tizen-hud-cli show-overlay --text "Searching content..." \
  && tizen-search-content-cli "Batman" \
  && tizen-hud-cli hide-overlay
```

## Getting Help

Each tool provides built-in help:
```bash
# General help
tizen-{tool-name}-cli --help

# Subcommand help
tizen-{tool-name}-cli {subcommand} --help

# JSON schema for programmatic discovery
tizen-{tool-name}-cli --schema
```

## Next Steps

1. **For CLI tools**: Refer to [cli/index.md](cli/index.md) and individual tool documentation
2. **For skills**: Refer to [skills/index.md](skills/index.md) and individual skill documentation
3. **For system commands**: Refer to [system_cli/index.md](system_cli/index.md) for categorized system tool documentation

This guide provides the foundation for effective tool usage. Individual tool and skill documentation contains detailed usage instructions and examples.
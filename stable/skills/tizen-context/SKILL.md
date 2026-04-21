---
name: tizen-context
description: Queries app/broadcast/HUD context, captures or browses screenshots by time, and gets key event history from Tizen TV. Subcommand required (context, screenshot, keys). Use screenshot subcommand with --at for time-relative screenshots (e.g., --at 10s). Requires agent-context-service running on device.
argument-hint: "[context|screenshot|keys]"
user-invocable: true
allowed-tools: true
---

# Tizen Tool Context

Query application/broadcast/HUD context, capture or browse screenshots by time, and get key event history from Tizen TV via `agent-context-service`.

**Prerequisite:** `agent-context-service` must be running on the device (port 50064).

## Intent-Based Workflow

| User says | Subcommand | Example |
|---|---|---|
| "지금 뭐 나와?", "what's on TV?" | `context` | `tizen-context-cli context` |
| "화면에 뭐가 보여?", "what's on screen?" | `screenshot --list --window 10s` | `tizen-context-cli screenshot --list --window 10s` |
| "방금 뭐 나왔어?", "what was on 10s ago?" | `screenshot --at` | `tizen-context-cli screenshot --at 10s` |
| "최근 화면들 보여줘" | `screenshot --list` | `tizen-context-cli screenshot --list` |
| "뭐 눌렀어?", "what keys were pressed?" | `keys` | `tizen-context-cli keys` |

**When to use `screenshot` vs `screenshot --capture`:**
- **`screenshot`** or **`screenshot --list --window 10s`** (no `--capture`): For **content recognition** — when the user asks about what's on screen, wants to identify content, or asks you to analyze the display. Uses continuously-tracked screenshots. Fast and non-intrusive. **Prefer `--list --window 10s`** over a single latest screenshot, because the user's thinking, typing, and speaking naturally takes several seconds — by the time their request reaches the agent, the screen may have already changed. A 10-second window captures what the user was actually looking at when they decided to ask.
- **`screenshot --capture`**: For **navigation and state confirmation** — when you need to verify the current UI state after key presses, confirm app transitions, or check focus position during UI navigation. Takes a guaranteed fresh capture.

## CLI Usage

```bash
tizen-context-cli [COMMAND] [OPTIONS]
tizen-context-cli --help
tizen-context-cli --schema
```

### Commands

| Command | Description |
|---|---|
| `context` | Query current app/broadcast/HUD context |
| `screenshot` | Get, capture, or list screenshots |
| `keys` | Get recent key event history |

A subcommand is required. Running without a subcommand prints help.

### Global Options

| Option | Description |
|---|---|
| `-h`, `--help` | Show help message |
| `--schema` | Print MCP-compatible JSON Schema Draft 7 and exit |
| `--address <addr>` | Override agent-context-service address (default: `localhost:50064`) |

### Screenshot Options

| Option | Description |
|---|---|
| `--capture` | Take a new screenshot on demand (saves PNG locally) |
| `--at <DURATION>` | Find screenshot closest to DURATION ago (e.g., `10s`, `5m`, `30`) |
| `-l`, `--list` | List recent screenshots (default count: 5) |
| `-c`, `--count <N>` | Number of screenshots to list (implies `--list`) |
| `-w`, `--window <DUR>` | Time window for list mode (e.g., `30s`, `5m`) |
| `-t`, `--type <TYPE>` | Screenshot type: `ui` or `content` |
| `--save-to <DIR>` | Save directory for `--capture` (default: `/tmp/screenshot`) |

**Screenshot modes:**
- **Default** (no flags): Returns the latest stored screenshot path
- **Capture** (`--capture`): Takes a new screenshot via agent-context-service, saves PNG, returns path
- **Lookup** (`--at 10s`): Finds the screenshot closest to 10 seconds ago
- **List** (`--list` or `--count N`): Lists N recent screenshots; combine with `--window` for time-based range

**Duration format:** `30` (seconds), `10s` (seconds), `5m` (minutes)

### Keys Options

| Option | Description |
|---|---|
| `-c`, `--count <N>` | Number of events to return (default: 50) |

Exit codes: `0` success, `1` runtime error, `2` usage error.

## Output

All output is a JSON envelope on stdout. Use `--schema` for the full machine-readable schema.

### context (default)

```json
{
  "status": "success",
  "tool": "tizen-context-cli",
  "message": "Current context: app=com.samsung.tv.csfs",
  "data": {
    "application": {
      "appId": "com.samsung.tv.csfs",
      "appName": "csfs",
      "appTitle": "Samsung TV Home",
      "packageName": "com.samsung.tv.csfs",
      "isForeground": true,
      "metadata": {}
    },
    "broadcast": null,
    "broadcastAvailable": false,
    "hud": {
      "recommendation": {
        "state": "ListShown",
        "focusedIndex": 0,
        "items": [
          {
            "title": "Batman Begins",
            "uri": "hud://item/0",
            "providers": [{ "name": "YouTube", "uri": "hud://item/0/provider/YouTube" }]
          }
        ]
      },
      "overlay": { "visible": false, "label": "" }
    }
  }
}
```

**How to act on `hud` context:**
- If `hud.recommendation.state` is `"ListShown"` and the user asks to play something from the list, use `tizen-hud-cli select <uri>` with the matching item's URI.
- If `hud.recommendation.state` is `"ListShown"` but the user's request is unrelated, first run `tizen-hud-cli hide-reco`, then proceed.
- If `hud` is `null`, agent-hud-service is not running — proceed normally.

### screenshot (default — latest)

```json
{
  "status": "success",
  "tool": "tizen-context-cli",
  "message": "Latest screenshot: /opt/usr/data/.../ui/20260325_143025.png",
  "data": {
    "mode": "latest",
    "screenshot": {
      "type": "ui",
      "filePath": "/opt/usr/data/.../ui/20260325_143025.png",
      "timestamp": "2026-03-25 14:30:25",
      "width": 640,
      "height": 360
    }
  }
}
```

### screenshot --capture

```json
{
  "status": "success",
  "tool": "tizen-context-cli",
  "message": "Screenshot saved: /tmp/screenshot/20260325_143052_123.png",
  "data": {
    "mode": "capture",
    "screenshot": {
      "type": "ui",
      "filePath": "/tmp/screenshot/20260325_143052_123.png",
      "timestamp": "2026-03-25 14:30:52",
      "width": 640,
      "height": 360
    }
  }
}
```

### screenshot --at 10s

```json
{
  "status": "success",
  "tool": "tizen-context-cli",
  "message": "Screenshot from ~10s ago: /opt/usr/data/.../ui/20260325_143015.png",
  "data": {
    "mode": "lookup",
    "requestedOffset": "10s",
    "screenshot": {
      "type": "ui",
      "filePath": "/opt/usr/data/.../ui/20260325_143015.png",
      "timestamp": "2026-03-25 14:30:15",
      "width": 640,
      "height": 360
    }
  }
}
```

### screenshot --list

```json
{
  "status": "success",
  "tool": "tizen-context-cli",
  "message": "5 recent screenshots",
  "data": {
    "mode": "list",
    "count": 5,
    "screenshots": [
      { "type": "content", "filePath": "...", "timestamp": "...", "width": 640, "height": 360 },
      { "type": "ui", "filePath": "...", "timestamp": "...", "width": 640, "height": 360 }
    ]
  }
}
```

### keys

```json
{
  "status": "success",
  "tool": "tizen-context-cli",
  "message": "20 key events",
  "data": {
    "count": 20,
    "events": [
      { "keyName": "Return", "timestamp": "2026-03-25 14:30:40.500", "appId": "com.samsung.tv.csfs", "isKeyDown": true },
      { "keyName": "Right", "timestamp": "2026-03-25 14:30:38.200", "appId": "com.samsung.tv.csfs", "isKeyDown": true }
    ]
  }
}
```

### Error

```json
{
  "status": "error",
  "tool": "tizen-context-cli",
  "code": "grpc_error",
  "message": "GetContext RPC failed: Connect Failed",
  "data": null
}
```

Error codes: `missing_argument`, `invalid_argument`, `unknown_command`, `invalid_duration`, `connection_error`, `grpc_error`, `screenshot_error`, `no_screenshot`, `file_error`, `not_supported`.

## When to Capture vs. Browse

- **`screenshot --list --window 10s`**: **Content recognition (recommended).** Use when the user asks about what's on screen, wants content identified, or asks you to analyze the display. The `--window 10s` captures what the user was actually looking at — their thinking, typing, and speaking naturally takes several seconds, so the screen may have changed by the time the request arrives. Analyzing multiple screenshots from the window gives the most complete picture. If the agent overlay is visible, hide it first:
  ```bash
  tizen-hud-cli hide-overlay && sleep 1 && tizen-context-cli screenshot --list --window 10s && tizen-hud-cli show-overlay --text "화면 분석 중..."
  ```
- **`screenshot`**: **Content recognition (simple).** Returns the single latest stored screenshot. Use when a quick glance is sufficient and you don't need the full time window.
- **`screenshot --capture`**: **Navigation and state confirmation.** Use when navigating app UI and you need to verify the current state — after key presses, app transitions, or to check focus position. Takes a guaranteed fresh capture. Slightly slower.
  ```bash
  tizen-aurum-cli key right && sleep 0.5 && tizen-context-cli screenshot --capture
  ```
- **`screenshot --at 10s`**: Use when the user asks "what was on screen a moment ago?" or you need to see what happened before the current state.
- **`screenshot --list --window 30s`**: Use when you need a longer sequence of screenshots to understand what happened over a period.

## Timing

Context from agent-context-service is continuously tracked. After major state changes:

| Action | Wait before querying | Reason |
|---|---|---|
| Channel change | 2-3 seconds | Broadcast metadata propagation |
| App launch / transition | 2-3 seconds | New app must fully load |
| Other UI actions | 1 second | Minor lag |

## Examples

```bash
# Current context
tizen-context-cli context

# Take a fresh screenshot
tizen-context-cli screenshot --capture

# Latest stored content screenshot
tizen-context-cli screenshot --type content

# Screenshot from ~10 seconds ago
tizen-context-cli screenshot --at 10s

# All UI screenshots from last 30 seconds
tizen-context-cli screenshot --list --window 30s --type ui

# Last 3 content screenshots
tizen-context-cli screenshot --count 3 --type content

# Recent key events
tizen-context-cli keys --count 20

# Print JSON schema for machine discovery
tizen-context-cli --schema
```

# Tizen Skills Index

This document provides an overview of all available skills in the tizen-tools package. Skills are high-level orchestrated workflows that combine multiple CLI tools to accomplish complex tasks.

## Available Skills

| Skill Name | Description | Primary Use Case | Documentation |
|------------|-------------|------------------|---------------|
| agent-browser | Browser automation CLI for AI agents — navigate, fill forms, click, screenshot, scrape, and authenticate via the agent-browser CLI | Programmatic web interaction and browser-based testing/automation | [SKILL.md](skills/agent-browser/SKILL.md) |
| tizen-content | Search and play content on Samsung TV. Orchestrates tizen-search-content-cli and tizen-play-content-cli | Content discovery and playback with recommendation UI | [SKILL.md](skills/tizen-content/SKILL.md) |
| tizen-context | Queries app/broadcast/HUD context, captures or browses screenshots by time, and gets key event history from Tizen TV | System context awareness and content recognition | [SKILL.md](skills/tizen-context/SKILL.md) |
| tizen-hud | Controls the agent-hud-service overlay and recommendation UI on Tizen TV via AppControl | UI overlay management and content selection | [SKILL.md](skills/tizen-hud/SKILL.md) |
| tizen-presentation-catalog | Generate presentation JSON for Samsung Tizen TV app with semantic TV presentation schema | Presentation JSON schema definition for TV viewer app | [SKILL.md](skills/tizen-presentation-catalog/SKILL.md) |
| tizen-ui-navigation | Navigates Tizen TV apps via remote keys and accessibility tree. Orchestrates tizen-aurum-cli, tizen-context-cli, and tizen-hud-cli with per-app guides | TV app UI navigation and on-screen interaction | [SKILL.md](skills/tizen-ui-navigation/SKILL.md) |
| tizen-viewer-launcher | End-to-end TV surface orchestration for natural-language display requests | Turn natural-language requests into TV surface payloads | [SKILL.md](skills/tizen-viewer-launcher/SKILL.md) |

## Skill Architecture

Skills are designed as orchestrated workflows that combine multiple CLI tools:

```
User Request → Skill → [CLI Tool 1] → [CLI Tool 2] → [CLI Tool N] → Result
```

### Key Characteristics
- **Intent-Based**: Skills interpret user intent and choose appropriate workflows
- **Orchestrated**: Combine multiple CLI tools for complex operations
- **Context-Aware**: Consider system state and user context
- **Pipelined**: Use stdin/stdout piping for efficient data flow
- **Error-Resilient**: Handle failures gracefully and provide meaningful feedback

## Detailed Skill Descriptions

### agent-browser
**Purpose**: Browser automation for AI agents via the `agent-browser` CLI (Chrome/Chromium over CDP)
- **Navigation**: Open URLs, wait for loads, handle redirects
- **Interaction**: Snapshot-based element refs (`@e1`, `@e2`) for clicking, filling, selecting
- **Capture**: Screenshots, full-page screenshots, video recording, PDF export
- **Data extraction**: Text/HTML scraping, structured snapshots
- **Authentication**: Import browser state, persist sessions, use user profiles, proxy support
- **References**: `references/{authentication,commands,profiling,proxy-support,session-management,snapshot-refs,video-recording}.md` for deep-dive topics
- **Templates**: `templates/{authenticated-session,capture-workflow,form-automation}.sh` for common workflows
- **Prerequisite**: Install the CLI via `npm i -g agent-browser`, `brew install agent-browser`, or `cargo install agent-browser`, then `agent-browser install` for Chrome

### tizen-content
**Purpose**: End-to-end content management for Samsung TV
- **Search**: Uses `tizen-search-content-cli` to find movies, TV shows, and YouTube videos
- **Play**: Uses `tizen-play-content-cli` to launch content via wascmd deeplinks
- **Recommend**: Uses `tizen-hud-cli` to show recommendation lists on TV
- **Workflow**: Intent-based decision between direct play vs. recommendation display

### tizen-context
**Purpose**: System awareness and content recognition
- **Context**: Query current app/broadcast/HUD state via `tizen-context-cli`
- **Screenshots**: Capture or browse screenshots for content analysis
- **Keys**: Get key event history for interaction tracking
- **Workflow**: Time-aware screenshot management and context interpretation

### tizen-hud
**Purpose**: UI overlay and content selection management
- **Overlay**: Show/hide text overlays on TV screen
- **Recommendations**: Display content recommendation lists
- **Selection**: Select and launch content from recommendation UI
- **State**: Query current HUD state and focused items

### tizen-presentation-catalog
**Purpose**: Presentation JSON schema definition for TV viewer app
- **Schema**: Defines required and optional fields for presentation JSON
- **Validation**: Works with `tizen-presentation-validate-cli` for payload validation
- **Output Rules**: Ensures clean JSON output without envelopes or markdown

### tizen-ui-navigation
**Purpose**: On-screen UI navigation for Tizen TV apps via remote keys and accessibility tree
- **Keys**: Uses `tizen-aurum-cli` to send remote keys (directional, enter, back, home, XF86 digits)
- **Inspection**: Uses `tizen-aurum-cli dump-tree` and `find` to read the focus-oriented widget tree
- **Verification**: Uses `tizen-context-cli` to confirm the foreground `appId` after launches/back presses
- **User feedback**: Uses `tizen-hud-cli` to show status overlays during multi-step navigation
- **Per-app guides**: Ships `apps/<appId>/README.md` (human-authored) and `apps/<appId>/NOTES.md` (AI-maintained) for Samsung TV Home (CSFS), YouTube, and Samsung TV Plus

### tizen-viewer-launcher
**Purpose**: Natural-language to TV surface orchestration
- **Intent Inference**: Maps user requests to appropriate TV domains
- **Domain Routing**: Routes weather, news, finance, schedule, and other domains
- **Presentation Generation**: Uses `tizen-presentation-catalog` for JSON generation
- **Delivery**: Uses `tizen-viewer-launch-cli` for TV app handoff

## Usage Patterns

### Content Workflow
```bash
# Search and play directly
tizen-search-content-cli "Batman" --youtube --limit 1 | tizen-play-content-cli

# Search and show recommendations
tizen-search-content-cli "Batman" --youtube --limit 3 | tizen-hud-cli show-reco --expand
```

### Context Workflow
```bash
# Get current app context
tizen-context-cli context

# Analyze recent screen content
tizen-context-cli screenshot --list --window 10s
```

### HUD Workflow
```bash
# Show overlay message
tizen-hud-cli show-overlay --text "Searching content..."

# Hide overlay
tizen-hud-cli hide-overlay

# Query current HUD state
tizen-hud-cli status
```

## Integration Points

### Pipe Support
Most skills support stdin/stdout piping for workflow chaining:
- JSON envelopes for structured data exchange
- Error propagation through pipe chains
- Automatic content selection and provider preference

### Service Dependencies
- **tizen-context**: Requires `agent-context-service` running on device
- **tizen-hud**: Requires `agent-hud-service` running on device
- **tizen-content**: Integrates with Samsung SearchAll API and YouTube

## When to Use Skills vs CLI Tools

### Use Skills When:
- You need orchestrated workflows combining multiple operations
- User intent requires interpretation and decision-making
- Complex error handling and fallback logic is needed
- UI interaction and state management is required
- Content discovery and recommendation workflows are needed

### Use CLI Tools When:
- You need direct, specific system interactions
- Simple, atomic operations are sufficient
- Maximum performance and minimal overhead are required
- Direct control over individual tool parameters is needed
- Debugging or testing specific functionality

For detailed usage of each skill, refer to the individual skill documentation linked in the table above.
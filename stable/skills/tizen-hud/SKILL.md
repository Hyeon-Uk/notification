---
name: tizen-hud
description: Controls the agent-hud-service overlay and recommendation UI on Tizen TV via AppControl. Use when you need to show/hide overlay text, content recommendations, query HUD state, or select content to launch on the TV screen.
argument-hint: <command> [OPTIONS]
user-invocable: true
allowed-tools: true
---

# Tizen Tool HUD

Control the agent-hud-service overlay and recommendation UI on Tizen TV.

## When to Use

- **Query HUD state** to see what content recommendations are showing and which item is focused
- **Select content** by URI to launch it with animated focus feedback on TV
- Show an **overlay text** on the TV screen (e.g., status messages, thinking indicator)
- Show **content recommendations** on the TV screen (search results with thumbnails and provider info)
- Hide overlay or recommendations

## When NOT to Use

- To **search** for content (use `tizen-search-content-cli` instead)
- To **navigate** TV UI (use `tizen-ui-navigation` instead)

## CLI Usage

```bash
tizen-hud-cli <command> [OPTIONS]
tizen-hud-cli --help                   # top-level help
tizen-hud-cli <command> --help         # subcommand-level help
tizen-hud-cli --schema                 # MCP-compatible JSON schema
```

Exit codes: `0` success, `1` runtime error, `2` usage error (bad flags, missing args, unknown command).

All output is a JSON envelope on stdout. Use `--schema` for the full machine-readable schema (JSON Schema Draft 7).

**Success example (show-reco):**
```json
{
  "status": "success",
  "tool": "tizen-hud-cli",
  "message": "Recommendation sent (3 items)",
  "data": {
    "action": "show-reco",
    "item_count": 3,
    "expand": true,
    "items": [
      {"title": "Batman Begins", "type": "umd_movie", "providers": ["wavve", "Apple TV"]},
      {"title": "The Batman Review", "type": "youtube", "providers": ["YouTube"]},
      {"title": "Batman", "type": "umd_movie", "providers": ["TVing", "wavve"]}
    ]
  }
}
```

**Error example:**
```json
{
  "status": "error",
  "tool": "tizen-hud-cli",
  "code": "empty_items",
  "message": "Pipe input contains no content items",
  "data": null
}
```

Error codes: `missing_argument`, `invalid_argument`, `unknown_command`, `empty_items`, `pipe_parse_error`, `upstream_error`, `app_control_error`.

### Commands

| Command | Description |
|---|---|
| `status` | Show current HUD state as JSON (recommendation list, focused item, overlay) |
| `select <uri>` | Select content item by URI from `status` output. Animates focus to the item on TV, then launches it |
| `show-reco [--items <json>] [-e, --expand]` | Show content recommendation. Reads JSON from stdin if `--items` not given. `-e`/`--expand` skips hint icon and shows content list directly |
| `hide-reco` | Hide content recommendation |
| `show-overlay --text <text>` | Show text overlay on screen |
| `hide-overlay` | Hide text overlay |

## When to Use `select`

Use `select` **only** when the user explicitly asks to play or open something from the recommendation list that is currently showing on screen. Examples:

- "Play the second one" → `status` to read list, then `select hud://item/1`
- "Play Batman Begins" → `status` to find matching title, then `select` its URI
- "Play that one on YouTube" → `select hud://item/N/provider/YouTube`

If the user's request is **not about the recommendation list** (e.g., "change the channel", "search for something else", "go to settings"), **hide the recommendation list first** with `hide-reco`, then proceed with the new task. Do not leave stale recommendations on screen.

## When to Use `hide-reco`

- User gives a new command unrelated to the recommendation list currently shown
- User explicitly asks to dismiss ("close that", "never mind")
- Before starting a new search/recommendation flow (old list will be replaced by `show-reco` automatically, but hiding first is cleaner)

## Querying HUD State and Selecting Content

After showing recommendations, use `status` to read the current state and `select` to launch content. This is the primary workflow for agent-driven content selection.

```bash
# Step 1: Show recommendations
tizen-search-content-cli "Batman" --limit 5 | tizen-hud-cli show-reco --expand

# Step 2: Read current state (see what items are showing)
tizen-hud-cli status
```

The `status` output includes URIs for each item and provider:

```json
{
  "recommendation": {
    "state": "ListShown",
    "focusedIndex": 0,
    "items": [
      {
        "title": "Batman Begins",
        "contentType": "umd_movie",
        "uri": "hud://item/0",
        "providers": [
          { "name": "YouTube", "license": "rental", "quality": "FHD",
            "uri": "hud://item/0/provider/YouTube" },
          { "name": "wavve", "license": "subscription", "quality": "4K",
            "uri": "hud://item/0/provider/wavve" }
        ]
      },
      {
        "title": "The Dark Knight",
        "contentType": "umd_movie",
        "uri": "hud://item/1",
        "providers": [
          { "name": "TVING", "license": "subscription", "quality": "FHD",
            "uri": "hud://item/1/provider/TVING" }
        ]
      }
    ]
  },
  "overlay": {
    "visible": false,
    "label": ""
  }
}
```

```bash
# Step 3: Select content — pick the item URI, optionally with a specific provider
tizen-hud-cli select hud://item/0/provider/YouTube   # specific provider
tizen-hud-cli select hud://item/0                     # auto-pick first provider
```

**Provider selection behavior:**
- `hud://item/N/provider/NAME` — launches the specified provider
- `hud://item/N` — auto-selects the first provider. For multi-provider items, the provider list still appears on screen with the first provider highlighted, so the user sees which one was chosen

When `select` is called, the TV shows an animated sequence: focus moves to the item, the provider highlights, holds briefly, then launches the content. The user sees exactly what the agent chose.

**Recommendation states:**
- `Hidden` — no recommendation UI visible
- `HintShown` — small pulsing hint icon at top of screen (waiting for user to press Info button)
- `ListShown` — full content card grid visible on screen

## Decision Flow

```
User prompt arrives
  │
  ├─ Related to shown recommendation list?
  │    ├─ YES: "play the 2nd one", "open Batman Begins", "play that on wavve"
  │    │    → tizen-hud-cli status
  │    │    → tizen-hud-cli select <uri>
  │    │
  │    └─ NO: "change channel", "search for cooking shows", "go home"
  │         → tizen-hud-cli hide-reco
  │         → proceed with new task
  │
  └─ No recommendation list showing?
       → proceed normally
```

## Pipe Support

`show-reco` reads a JSON envelope (or legacy raw array) from stdin when `--items` is not provided. Pipe search results from `tizen-search-content-cli` directly. If upstream reports an error, it is propagated. See `tizen-search-content-cli` skill for the full output format and available search options (YouTube, filters, etc.).

```bash
# Pipe single search
tizen-search-content-cli "Batman" | tizen-hud-cli show-reco

# Pipe merged searches with filters
tizen-search-content-cli "Batman" --limit 3 \
  | tizen-search-content-cli "Superman" --limit 2 \
  | tizen-hud-cli show-reco

# Mix YouTube and regular search results
tizen-search-content-cli "Batman trailer" --youtube --limit 2 \
  | tizen-search-content-cli "Batman" --limit 3 \
  | tizen-hud-cli show-reco

# Rich recommendation: YouTube reviews + movies + related actor
tizen-search-content-cli "Interstellar review" --youtube --limit 2 \
  | tizen-search-content-cli "Interstellar behind the scenes" --youtube --time year --limit 1 \
  | tizen-search-content-cli "Interstellar" --limit 3 \
  | tizen-search-content-cli "Christopher Nolan" --limit 2 \
  | tizen-hud-cli show-reco

# --expand: skip hint icon, show content list directly
# Use when the user explicitly asked to search/find content
tizen-search-content-cli "Batman" --limit 5 | tizen-hud-cli show-reco --expand
```

## Overlay with Screenshot / dump-tree

The overlay is rendered on top of the TV screen. When taking a screenshot or dump-tree, hide the overlay first to capture the actual screen content, then show it again with a contextual status message.

**Screenshot (content recognition)** — use `screenshot --list --window 10s` to capture what the user was watching (their thinking/typing takes seconds, so a window gives the most complete picture):
```bash
tizen-hud-cli hide-overlay && sleep 1 && tizen-context-cli screenshot --list --window 10s && tizen-hud-cli show-overlay --text "화면 분석 중..."
```

**Screenshot (navigation/state check)** — use `screenshot --capture` for verifying UI state during navigation:
```bash
tizen-hud-cli hide-overlay && sleep 1 && tizen-context-cli screenshot --capture && tizen-hud-cli show-overlay --text "화면 확인 중..."
```

**dump-tree** — wait 1s after hiding so the overlay widget is removed from the UI tree:
```bash
tizen-hud-cli hide-overlay && sleep 1 && tizen-aurum-cli dump-tree && tizen-hud-cli show-overlay --text "화면 분석 중..."
```

Use **natural, conversational Korean** with emoji. Write as if you're talking to the person watching TV — friendly and human, not technical.

| OK | NG |
|--------|-------------|
| `"핑크퐁 틀어줄게요! 🎵"` | `"핑크퐁 채널로 이동 중..."` |
| `"뉴스 바꿔드릴게요 📺"` | `"KBS 뉴스24 채널로 이동 중..."` |
| `"Xbox 켜줄게요! 🎮"` | `"Xbox 앱으로 이동 중..."` |
| `"Spotify 실행할게요 🎵"` | `"Spotify 앱 실행 완료 ✓"` |
| `"지금 화면 보고 있어요 👀"` | `"화면 내용 인식 중..."` |
| `"관련 영상 찾고 있어요 🔍"` | `"유튜브 영상 검색 중..."` |
| `"잠깐만요, 바꿔드릴게요 ⏳"` | `"이동 중..."` |
| `"다 됐어요! ✅"` | `"완료 ✓"` |

**Key rule:** TV 앞에 있는 사람이 자연스럽게 읽을 수 있는 말투로. 채널명/앱명을 그대로 쓰되 "틀어줄게요", "바꿔드릴게요", "찾고 있어요" 같은 친근한 동사와 이모지를 붙여야 함.

## Examples

```bash
# On-device: query HUD state
tizen-hud-cli status

# On-device: select content by URI (specific provider)
tizen-hud-cli select hud://item/0/provider/YouTube

# On-device: select content by URI (auto-pick provider)
tizen-hud-cli select hud://item/2

# On-device: hide recommendation (user moved on to a different task)
tizen-hud-cli hide-reco

# On-device: show overlay
tizen-hud-cli show-overlay --text "검색 중..."

# On-device: hide overlay
tizen-hud-cli hide-overlay

# On-device: pipe search results to recommendation
tizen-search-content-cli "Batman" | tizen-hud-cli show-reco

# On-device: show recommendation with inline JSON
tizen-hud-cli show-reco --items '[{"id":"1","title":"Batman","thumbnail_url":"...","providers":[]}]'

# On-device: hide recommendation
tizen-hud-cli hide-reco
```

## Content Item JSON Format

The `show-reco` command expects a JSON array of content items:

```json
[
  {
    "id": "0",
    "title": "Batman Begins",
    "thumbnail_url": "https://...",
    "content_type": "umd_movie",
    "providers": [
      {
        "name": "wavve",
        "app_id": "abc123",
        "icon_url": "https://...",
        "deeplink": "movie?id=123",
        "license": "rental",
        "quality": "FHD"
      }
    ]
  }
]
```

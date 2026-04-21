---
name: tizen-ui-navigation
description: "Navigates Tizen TV apps via TV remote keys and accessibility tree. Use when the user wants to launch or install an app, switch category tabs (추천/라이브/Apps), tune a Samsung TV Plus channel, browse home-screen content rows, find an on-screen item, or otherwise operate the TV UI. Orchestrates tizen-aurum-cli (keys/clicks/dump-tree/find), tizen-context-cli (verify foreground app), and tizen-hud-cli (status overlay). Includes per-app guides for Samsung TV Home (CSFS), YouTube, and Samsung TV Plus."
user-invocable: false
allowed-tools: true
---

# Tizen UI Navigation

Per-application navigation guides for Tizen TV. Each app has its own documentation describing screen layouts, navigation flows, and key UI elements.

## Workflow Overview

A navigation session follows this sequence:

1. **Identify the current app** — run `tizen-context-cli` to get the foreground `appId`.
2. **Look up the navigation guide** — read `apps/<appId>/README.md` and `apps/<appId>/NOTES.md`.
3. **Orient** — run `dump-tree` once to survey the screen structure.
4. **Plan the path** — use the README's screen layout and recipes to determine the shortest key sequence.
5. **Navigate** — send key presses via `tizen-aurum-cli`, batching deterministic sequences.
6. **Verify** — use `get-focused`, `dump-tree --id`, or `tizen-context-cli` at critical decision points only.
7. **Update notes** — after the session, update `NOTES.md` with new insights.

## How to Use

Each app folder contains two files:

| File | Author | Purpose |
|---|---|---|
| `README.md` | Human | Canonical navigation guide — read-only for AI agents |
| `NOTES.md` | AI agent | Discovered insights — structured, size-capped |

Look up the target application by its appId:

```
skills/tizen-ui-navigation/apps/<appId>/README.md
skills/tizen-ui-navigation/apps/<appId>/NOTES.md
```

## Before Navigating

Before starting any navigation session with an application:

1. **Read both files** for the target app — `README.md` for the canonical guide, and `NOTES.md` for AI-discovered tips and known issues. The NOTES.md file contains critical caveats like element ID recycling behavior that are essential for correct navigation.
2. **Use only the operations listed** in the app's README.md. Do not attempt to use CLI tools (pkgcmd, app_launcher, sdb, etc.) for tasks like launching or installing apps — these require elevated permissions. The TV UI is the primary interface.
3. **Follow the timing model** — see the Timing section below.

## HUD Overlay During Navigation

Show overlay text **before** starting a navigation action so the user knows what's happening. Use natural, conversational Korean with emoji.

```bash
# 탭 이동
'tizen-hud-cli show-overlay --text "앱 탭으로 이동하는 중이에요 📱"'
'tizen-hud-cli show-overlay --text "라이브 탭으로 이동하는 중이에요 📺"'
'tizen-hud-cli show-overlay --text "홈 화면으로 돌아가는 중이에요 🏠"'

# 앱 찾기 / 실행
'tizen-hud-cli show-overlay --text "Xbox 앱 찾고 있어요 🎮"'
'tizen-hud-cli show-overlay --text "Spotify 앱으로 이동하는 중이에요 🎵"'
'tizen-hud-cli show-overlay --text "앱 목록에서 찾고 있어요 🔍"'

# 채널 / 콘텐츠
'tizen-hud-cli show-overlay --text "뽀로로 채널 틀어줄게요 🐧"'
'tizen-hud-cli show-overlay --text "뉴스 채널로 바꿔드릴게요 📰"'

# 완료
'tizen-hud-cli show-overlay --text "Xbox 켜졌어요! 🎮"'
'tizen-hud-cli show-overlay --text "다 됐어요! ✅"'
```

> **Rules:**
> - Always `hide-overlay` before `dump-tree` or screenshot (sleep 1s after hide)
> - Use `--bash` when chaining multiple commands with `&&` or pipes

## Navigation Strategy

Follow these principles before and during every navigation session.

### 1. Plan the path before acting

Before pressing any key, determine:
- **What is the goal?** (e.g., "launch the Xbox app", "find a sports channel")
- **Where is the target?** (which screen, tab, row, or area contains it)
- **What is the shortest path from the current focus to the target?**

Consult the app's README.md to identify the correct tab and row. If the target location is known, go directly — see "navigate or search" in step 3.

### 2. Use lightweight reconnaissance

Gather only the information you need, using the cheapest method first:

| Method | Cost | When to use |
|---|---|---|
| `dump-tree --focused` | Cheapest | Identify current focus and what `enter` would activate |
| `get-focused` | Cheapest | Determine current position before planning a path |
| `dump-tree --id <id>` | Low | Inspect a specific row or container |
| `dump-tree` | Low | Survey overall screen structure. Run once at session start. |
| `find --text "target"` | Low | Search for element. **Unreliable in NUI apps** — never conclude absence from empty results. |
| `dump-tree --raw` | High | Only when filtered output is insufficient |
| Screenshot | Expensive | Visual verification when dump-tree is insufficient |

> ⚠️ **HUD overlay must be hidden before dump-tree or screenshot.**
> If the overlay is visible, aurum returns the HUD widget tree (not the app UI), producing `focused: none` and empty results. Also hide before screenshots.
>
> ```bash
> # ✅ dump-tree with overlay management
> tizen-hud-cli hide-overlay && sleep 1 && tizen-aurum-cli dump-tree && tizen-hud-cli show-overlay --text "화면 구조 파악 중이에요 🔍"
>
> # ✅ screenshot with overlay management
> tizen-hud-cli hide-overlay && sleep 1 && tizen-context-cli screenshot --capture && tizen-hud-cli show-overlay --text "지금 화면 보고 있어요 👀"
> ```

### 3. Choose the right mode: navigate or search

**Known target** (e.g., "launch YouTube from Apps tab"): Count minimum key presses from documentation, execute in a batch, verify once.

**Unknown target** (e.g., "find a war movie"): Press `down` once, check the row label, repeat. Do not batch — each row could be the target. Start by scanning the initial `dump-tree` for matching row labels; if not found, switch to one-row-at-a-time search.

### 4. Stop and reassess on unexpected results

If a key press produces unexpected results:
1. **Stop immediately** — do not retry the same action.
2. Run `dump-tree` or `get-focused` to determine actual state.
3. Identify why the result differed.
4. Revise the plan from the confirmed position.

### 5. Verify app after launching

After pressing `enter` on any item that should launch an app, run `tizen-context-cli` (after appropriate delay) to confirm the correct app is in the foreground.

### 6. Follow task-oriented recipes

For common tasks, follow the recipes in each app's README.md. Always check for a matching recipe before planning your own path.

## dump-tree Behavior

- Default output is navigation-optimized, filtered by focusable + clickable, with a focus summary header (`--- focused ---` block). Within `[items]` containers, items are in visual left-to-right order with focused item marked `>>>` at position `[0]`. Distances like `[+3]`/`[-2]` show key presses needed.
- `dump-tree --focused` outputs only the focused subtree (~10-20 lines) — cheapest reconnaissance.
- `dump-tree --id <id>` scopes to a specific container (~20-35 lines).
- `dump-tree --raw` outputs full JSON (~1000+ lines) — use only when filtered output is insufficient.

## Timing

| Action | Delay after | Reason |
|---|---|---|
| Key press (up/down/left/right) | 0.5s | UI animation and focus update |
| Tab activation (enter on Category) | 1s | Content area reloads |
| App launch (enter on app tile) | 3s | App binary loads |
| Back / Home key | 2-3s | Previous app UI reloads, element IDs regenerate |
| App install | Poll | Poll with `get-focused` or `dump-tree` until button changes to "실행" (Launch) |
| Number key digit entry (`key --xf86 <digit>`) | 0.3s between digits | TV digit input timeout is ~0.5s |
| Before `tizen-context-cli` | 3s after channel change; 1s otherwise | Broadcast metadata lags |

## Troubleshooting

### Key press returns error
Retry once after 1s. If it fails again, send any `dump-tree` command — aurum auto-relaunches the bootstrap service.

### `tizen-context-cli` returns stale data
Check `broadcast.metadata.DataValid`. If `"0"`, wait 3s and retry. After first entering Samsung TV Plus, context may remain empty until you send one channel change and navigate back.

### `find --text` returns no results
Do **not** conclude the item is absent. Fall back to `dump-tree --id <container>` and scan items.

### `dump-tree` returns empty output
UI hasn't finished loading. Wait the full duration from the timing table and run `dump-tree` once.

### App crash or unexpected screen
Run `tizen-context-cli` to identify the current app. If unexpected, press `key home` and restart navigation.

## Updating AI Notes

After completing a navigation session, update `NOTES.md` for the app:

1. **Read before write** — avoid duplicating existing content.
2. **Replace, don't append** — consolidate rather than add to the bottom.
3. **Respect section limits** — drop least valuable entries when at limit.
4. **One update per session**.

### NOTES.md template

When `NOTES.md` does not exist for an app, create it using this template:

```markdown
# AI Navigation Notes — <appId>

## UI Structure
<!-- Max 60 lines. Replace entirely when updating. -->

## Tips
<!-- Max 20 items. One line each. Replace weak tips with better ones. -->

## Known Issues
<!-- Max 10 items. Remove resolved issues. -->
```

## First-time App Navigation

When navigating an app with no existing guide:

1. **Identify the app type** — use `tizen-context-cli` to get the appId, then check if `dump-tree` returns a meaningful widget tree.
   - If it does: native NUI/DALi app. Proceed with aurum commands.
   - If empty/minimal: likely a web app. Only `tizen-context-cli`, screenshots, and XF86 keys will work.

2. **Run `dump-tree`** to understand screen layout. Note container hierarchy, focusable elements, and positions.

3. **Create `NOTES.md`** using the template above.

4. **Navigate cautiously** — verify after each key press until you understand the UI behavior. Once confident, batch key presses.

## Applications

| App ID | Description |
|---|---|
| `com.samsung.tv.csfs` | Samsung TV Home (CSFS) — native NUI app. Browse recommended OTT content, curated live TV channels, and installed applications. Use Applications tab to find/install/launch apps. Use Live tab for Samsung TV Plus channels. |
| `com.samsung.tv.cobalt-yt` (111299001912) | YouTube — web app. **Prefer `tizen-search-content-cli` + `tizen-play-content-cli`** to play content. dump-tree does not work; use screenshots and directional keys only when the user explicitly asks to navigate the on-screen UI. |
| `HEPsqFNie0.tvplusstandalone` | Samsung TV Plus — web app (free live TV). **Prefer `tizen-play-content-cli tvplus_channel <number>`** (e.g., `tizen-play-content-cli tvplus_channel 503`) to tune to a channel — it launches the app and sends digit keys with proper delays. Only fall back to manual navigation (`key --xf86 <digit>`) if the play tool is unavailable. Channel list in NOTES.md. |

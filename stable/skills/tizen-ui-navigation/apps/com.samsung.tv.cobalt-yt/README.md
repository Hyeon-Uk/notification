# com.samsung.tv.cobalt-yt (111299001912)

YouTube application on Tizen TV.

## General Rules

- **App type**: Web application — `tizen-aurum-cli dump-tree` does **not** work. Do not attempt dump-tree, get-focused, or find commands.
- **Visual navigation only**: Use `tizen-context-cli screenshot --capture` to see the current screen state.
- **Allowed operations**:
  - `tizen-aurum-cli key left` / `right` / `up` / `down` — move focus
  - `tizen-aurum-cli key enter` — select / play
  - `tizen-aurum-cli key back` — go back / close overlay
  - `tizen-aurum-cli key XF86Info` — show video info overlay (title, progress, recommendations)
  - `tizen-context-cli screenshot --capture` — visual confirmation of screen state

## Playing YouTube Content

### Search and play (preferred)

The best way to play YouTube content is to use the search and play tools:

```bash
# Search and play directly
tizen-search-content-cli "cats" --youtube --limit 1 | tizen-play-content-cli

# Or specify a known video ID
tizen-play-content-cli youtube dQw4w9WgXcQ
```

This launches YouTube and plays the content directly without any UI navigation. Always prefer this approach unless the user explicitly asks to interact with what is already on screen.

### Navigate on screen (when explicitly requested)

When the user asks to play content visible on the YouTube screen (e.g., "play the next one", "find an economics video in the recommendations"), navigate the UI manually:

1. Take a screenshot to see the current screen layout.
2. Use directional keys (`left`, `right`, `up`, `down`) to move focus to the target.
3. Take screenshots between key presses to track focus position.
4. Press `enter` to select / play.

Since dump-tree is unavailable, screenshots are the only way to understand the UI state. Take a screenshot after every few key presses to confirm focus has moved as expected.

#### ⚠️ Horizontal list scrolling — visual illusion

When navigating left/right within a row, the **entire list slides** in the opposite direction of the key press. The focused item always appears at the **leftmost visible position** — the highlight does not visually move right.

- Pressing `right` once: 2nd item is focused, but highlight is still at the leftmost card
- Pressing `right` N times: (N+1)th item is focused, highlight still at leftmost position
- **Do not mistake this for a stuck focus** — the focus is moving correctly; the list is scrolling under it

To identify which item is focused: look at the **enlarged/expanded card at the leftmost position** in the row — that is the currently selected item.

**Counting tip**: Track how many times you pressed `right` from the start. After N presses, focus is on item N+1 (1-indexed). Do not rely on visual highlight position alone.

### Screenshots during video playback

When a video is playing, the screen shows only the video — no title, no UI elements. To see video info and recommendations, **press the Info key first**:

```bash
# Hide HUD overlay (if shown) → press Info → capture before it disappears
tizen-hud-cli hide-overlay && tizen-aurum-cli key XF86Info && sleep 0.5 && tizen-context-cli screenshot --capture
```

The info overlay shows the video title, progress bar, and recommended content below. It auto-dismisses after a few seconds, so always chain the screenshot command to avoid losing the view.

### Play next recommended content during playback

When the user wants to play the next/related content while watching a video:

```bash
# Info → move to recommendations → play first recommended item
tizen-aurum-cli key XF86Info && sleep 0.5 && tizen-aurum-cli key down && sleep 0.3 && tizen-aurum-cli key enter
```

This opens the info overlay, moves focus down to the first recommended item, and selects it.

### Play a specific item by position

When the user clearly specifies an item position (e.g., "play the 3rd item in the 2nd row"), chain the key presses into a single command ending with `enter`. Only do this when the user's prompt is unambiguous about the position.

```bash
# Example: play 3rd item in 2nd row (from current focus at top-left)
# down to 2nd row → right 2 times to 3rd item → enter
tizen-aurum-cli key down && sleep 0.3 && tizen-aurum-cli key right && sleep 0.3 && tizen-aurum-cli key right && sleep 0.3 && tizen-aurum-cli key enter
```

If the position is ambiguous or you are unsure where the current focus is, take a screenshot first to confirm before chaining.

## Timing

| Action | Delay after | Reason |
|---|---|---|
| Key press (directional) | 0.3s | UI focus animation |
| Enter (play content) | 3s | Video loading |
| Back | 2s | UI transition |
| XF86Info | 0.5s | Info overlay animation |
| Screenshot | 1s after hide-overlay | Ensure overlay is cleared |

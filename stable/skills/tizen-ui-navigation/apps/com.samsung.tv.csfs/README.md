# com.samsung.tv.csfs

Samsung TV Home application (CSFS). This is the main home screen that provides content recommendations, live TV channels, and application shortcuts.

> **Note:** Row labels and their contents may vary whenever the home application's content is updated by Samsung. The rows listed in each tab section below reflect a specific observation and may differ across regions, firmware versions, or content refresh cycles.

## General Rules

- **Timing**: Follow the timing model in `tizen-ui-navigation/SKILL.md`. Key presses need 0.5s delay; tab activations need 1s; app launches need 3s; returning home needs 2-3s.
- **Allowed operations**: Only use the following `tizen-aurum-cli` commands:
  - `dump-tree` — inspect the current UI state
  - `key up`, `key down`, `key left`, `key right` — directional navigation
  - `key enter` — select / activate the focused item
  - `key back` — go back
  - `key home` — return to home

  These operations are sufficient to navigate the entire home application.

## CSFS-specific Navigation Notes

### Confirming which list is focused

When `dump-tree` shows a container with `{focused}` and its child `[items]` list contains `[0] >>>`, that list is already focused. Proceed directly with `left`/`right` to find the target item.

### Navigating to distant items

Only items near focus are on-screen (marked `clickable` in `dump-tree`). To reach a distant item, keep pressing `left`/`right` and re-dump with `dump-tree --id <items-id>` after each step.

### Off-screen lists

Some lists may appear empty. Scroll with `down`/`up` to bring them into view — items populate once visible.

## Screen Layout

### Top controls

The top of the screen contains four controls: **SSO**, **Setting**, **Category**, and **PrivacyChoice**.

- Send `up` repeatedly to move focus up to the top controls.
- Use `left` / `right` to switch focus between them.
- Send `down` to return to the content section below.

### Category tabs

The **Category** control contains three tab items:

| Tab | Korean label | Description |
|---|---|---|
| Recommendation | 추천 | Content recommendations from providers such as Netflix, Samsung TV Plus, TVING, Apple TV, YouTube, and others |
| Live | 라이브 | Live TV channel recommendations and shortcuts, primarily from Samsung TV Plus |
| Applications | Apps | Installed and available applications |

Category order: 추천 → 라이브 → Apps. Unlike content lists (where pressing past the end has no effect), pressing `right` past Apps **escapes** to **PrivacyChoice** and pressing `left` past 추천 **escapes** to **Setting**. Always check which tab is `[0] >>>` before pressing directional keys — do not overshoot.

To switch categories:
1. Navigate to the **Category** control (send `up` until focused).
2. Run `dump-tree` or `get-focused` to check which tab is currently `[0] >>>`.
3. Calculate the exact number of `right` / `left` presses needed. If the target tab is already focused, skip to step 5.
4. Press `right` / `left` exactly N times — do not overshoot.
5. Send `enter` to activate the selected category.
6. Focus automatically moves to the first focusable row or control in the new tab — do **not** press `down` to enter the content area.

### Bottom button (jump to top)

Every tab has a 72x72 button at the bottom of the scroll area (position ~(924, 40), text pattern `s/terra.bottom@d/<tab-id>`). Pressing `enter` on this button jumps focus back to the **Category** bar — useful for quickly returning to the top from deep in the content area.

## Recommendation Tab

The Recommendation tab presents content from various streaming providers. Row labels and contents vary as the home application's content is refreshed.

### First content row

The first row ("지금 뜨는 콘텐츠", 1920x328) features a pinned **삼성 TV 플러스** card plus a carousel of trending content (~20 items). Selecting a content item opens `com.samsung.tv.ContentsBrowserDetail`, a detail page that may offer the content through third-party providers (e.g. Watcha, Netflix) rather than Samsung TV Plus directly.

### Content rows

Below the first row, content is organized into themed rows (each 1920x352 unless noted). Observed rows include:

- **Genre rows**: 코미디, 액션 스릴러, 다큐멘터리, 드라마, 전쟁, and themed collections (e.g. "대담한 탐험", "미래 비전 탐구", "환상적인 여정")
- **Curated rows**: "한국 Top 10 영화" (1920x424, taller format with ~568x320 tiles — items have no inline text, but the focused item's title appears in the description label `:1.XX:142`), "무료 영화", "TVPlus 음악"
- **Live channel rows**: "24/7 전체 영화 채널", "바로 지금! 실시간 핫한 채널", "곧 떠날 무료 영화 시청하기"
- **Sponsored Row**: advertisement tiles
- **BubbleTab** (1920x256): an inline category selector for filtering content
- **Genre Bookmark** (1920x256): genre shortcut tiles
- **미디어 옵션** (1920x288): settings/preferences row

### Launcher row

Near the bottom, a small launcher row (1920x112) contains app shortcut tiles (112x112 each).

## Live Tab

The Live tab shows live TV channel content, primarily from Samsung TV Plus.

### Banner

The top of the Live tab shows a large preview banner (1920x528) of a currently-airing Samsung TV Plus channel. It includes:
- Channel name and number (e.g. "삼성 TV 플러스, 502, 연합뉴스TV")
- A progress bar
- A **바로 보기** (Watch Now) button

Pressing `enter` on the banner launches Samsung TV Plus (`HEPsqFNie0.tvplusstandalone`). The transition goes through `org.tizen.tv-viewer` before landing in the standalone TV Plus web app.

### Content rows

Below the banner, content rows (each 1920x352 unless noted) include:
- **현재 방송 중** (1920x328) — currently broadcasting shows. Standard 456x256 tiles.
- **바쁜 현대인 집중! 핫 클립 채널** — hot clip channels
- **믿고 보는 장수 예능!** — long-running variety shows
- **OnNow Sports** — live sports content
- **Samsung TV Plus : Live Events** — live events

Additional rows may appear depending on content updates.

### EPG channel row

The bottom of the Live tab has an EPG row (1920x296) with genre filter tiles (448x112 each). The row contains 12 genre categories:

| Category | Korean label |
|---|---|
| Live | 실시간 |
| News | 뉴스 |
| Variety | 예능 |
| Drama | 드라마 |
| Kids | 어린이 |
| Current Affairs / Culture | 시사/교양 |
| Lifestyle | 라이프스타일 |
| Tech / Games / Science | 기술/게임/과학 |
| Sports | 스포츠 |
| Shopping | 쇼핑 |
| Music | 음악 |
| Movies | 영화 |

The EPG row aggressively recycles element IDs — the same container shows different content as you scroll. Navigate with `left` / `right` and re-dump after each step to see the current genre.

## Applications Tab

The Applications tab shows installed and available apps.

### Row layout

The tab contains a non-focusable welcome banner ("앱스에 오신 것을 환영합니다", 1920x360) at the top, followed by 9 content rows:

| Row | Korean label | Tile type | Tile size |
|---|---|---|---|
| RECOMMENDED | 삼성 제공 / 추천 | Small icon | 112x112 |
| EDITORSCHOICE | 에디터 추천 | Large card | 416x368 |
| DOWNLOADED | 다운받은 앱 | Small icon | 112x112 |
| LIFESTYLEAPPSWW | 라이프스타일 앱 | Large card | 416x368 |
| POPULAR_THIS_WEEK | 이번 주 인기 앱 | Small icon | 112x112 |
| BESTAPPS | 최신 앱 | Large card | 416x368 |
| GROUPCONTENT | 그룹 콘텐츠 | Group card | 456x152 |
| GENRE | 장르 | Genre card | 456x152 |
| SETTING | 설정 | Setting tile | 176x192 |

### Row purposes

Each row serves a specific purpose. Use this to jump directly to the right row for a given task:

| Task | Target row | Down presses from initial focus |
|---|---|---|
| Launch an installed app | **DOWNLOADED** | 2 |
| Find a popular/trending app | **EDITORSCHOICE** or **POPULAR_THIS_WEEK** | 1 or 4 |
| Find a Samsung system app | **RECOMMENDED** (left section) | 0 (initial focus) |
| Find an app by genre/category | **GENRE** | 7 |
| Browse lifestyle/utility apps | **LIFESTYLEAPPSWW** | 3 |
| Browse newest apps | **BESTAPPS** | 5 |

**Key insight**: The DOWNLOADED row contains **every installed app** and is only 2 `down` presses from the initial focus after entering the Applications tab. When launching an app that is already installed, always check DOWNLOADED first — do not scroll past it to search in other rows.

### Tile types

- **Small icon tiles** (112x112): app icon only on screen (no visible text label). However, `dump-tree` output includes the app name as text (e.g., `[Control] "YouTube" @(...) id=...`). Used for Samsung-provided, recommended, downloaded, and popular apps.
- **Large app cards** (416x368): 416x272 image area + 416x96 info area with app name and install status ("설치됨" = installed). The focused card gets a highlight overlay.
- **Group content cards** (456x152): group name and short description (e.g. "음악 & 라디오", "심심풀이용 앱", "편리한 생활").
- **Genre content cards** (456x152): genre label tiles (동영상, 스포츠, 게임, 라이프스타일, 정보, 교육).
- **Setting option tiles** (176x192): settings controls (자동 업데이트, 모두 업데이트, 앱 설정).

### RECOMMENDED row structure

The RECOMMENDED row has a divider separating two sections: "삼성 제공" (Samsung Provided) on the left with system apps (Bixby, SmartThings, etc.) and "추천" (Recommended) on the right with popular streaming/media apps (Netflix, YouTube, Spotify, etc.).

## Entering Samsung TV Plus

There are two paths from the Home screen to Samsung TV Plus. **Prefer the Live tab** when the goal is to watch a specific channel — it provides a reliable, direct entry point.

### Path 1: Live tab (recommended for channel tasks)

1. Navigate to the **라이브 (Live)** tab (see "Switch between category tabs").
2. The top banner shows a currently-airing Samsung TV Plus channel with a **바로 보기** (Watch Now) button.
3. Press `enter` on the banner to launch Samsung TV Plus.
4. Verify with `tizen-context-cli` after 3 seconds — confirm `appId` is `HEPsqFNie0.tvplusstandalone`.
5. Once inside, use `key --xf86 <digit>` for direct channel entry.

### Path 2: Recommendation tab

1. From the **Recommendation** tab (default), the first content row has a pinned **삼성 TV 플러스** card that is typically the focused item.
2. Run `dump-tree --focused` to confirm the **삼성 TV 플러스** card is indeed focused before pressing `enter`.
3. Press `enter` to launch Samsung TV Plus.
4. Verify with `tizen-context-cli` after 3 seconds — confirm `appId` is `HEPsqFNie0.tvplusstandalone`.

**Caution**: The Recommendation tab's first row content may vary after content refreshes. The **삼성 TV 플러스** card is usually but not always the focused item. Always verify focus before pressing `enter`.

The app transitions: `com.samsung.tv.csfs` → `org.tizen.tv-viewer` → `HEPsqFNie0.tvplusstandalone`. Once inside Samsung TV Plus, use `tizen-context-cli` for channel info, `key --xf86 <digit>` for direct channel entry, or `XF86RaiseChannel` / `XF86LowerChannel` for sequential browsing. See the `HEPsqFNie0.tvplusstandalone` guide for details.

## Common Task Recipes

### Launch an installed app

The target is always the **DOWNLOADED** row in the Applications tab.

**From the home screen (any tab):**

1. `get-focused` — determine current position.
2. Press `up` until focus reaches the **Category** control.
3. Press `right` until the **Apps** tab is `[0] >>>`. Press `enter`.
4. Focus lands on the first row (RECOMMENDED). Press `down` twice to reach **DOWNLOADED**.
5. Use `dump-tree --id <downloaded-items-id>` to see the visible apps — the focused item is marked `>>>` at `[0]`.
6. Press `left` / `right` to find the target app.
7. Press `enter` to launch.

**Typical total**: 4 presses to reach Apps tab + 2 down + a few left/right + enter = **~7-10 key presses**.

### Install and launch a new app

1. Follow steps 1-3 above to reach the Applications tab.
2. Navigate to the row most likely to contain the app (see **Row purposes** table above).
3. Press `left` / `right` within the row to find the target app.
4. Press `enter` to open the app's store detail page.
5. Focus lands on the Install button. Press `enter` to install.
6. After installation completes, the button changes to "실행" (Launch). Press `enter` to launch.

### Search for content by genre or topic

When the target row is unknown (e.g., "find a war movie"):

1. Navigate to the **Recommendation** tab (see "Switch between category tabs" below).
2. Run `dump-tree` — scan row labels for your keyword (e.g., "전쟁" for war). Genre rows are **lazy-loaded** and may not appear until scrolled into view.
3. If found → count rows from current position and navigate directly.
4. If not found → press `down` one row at a time, checking each row's label with `dump-tree --id` or `get-focused`. Do **not** batch presses during search.
5. Some rows have items with no inline text (e.g., "한국 Top 10 영화"). Use the description label (`:1.XX:142`) to read the focused item's title, then `left` / `right` to scan other items.

### Watch a Samsung TV Plus channel

When the user asks to watch a TV channel:

1. **Look up the channel** — check the channel list in `HEPsqFNie0.tvplusstandalone/NOTES.md` to find the channel number. If the channel name is ambiguous, confirm with the user.
2. **Enter Samsung TV Plus** — from the Recommendation tab, press `enter` on the **삼성 TV 플러스** card (typically the focused item on the first row). Wait 3 seconds for the app to load.
3. **Bootstrap context** — after entering TV Plus, `tizen-context-cli` may return empty data. Send one `XF86RaiseChannel`, wait 3 seconds, then proceed.
4. **Jump to the channel** — send each digit of the channel number as `key --xf86 <digit>` with 0.3s delays between digits. Wait 3 seconds.
5. **Verify** — read `tizen-context-cli` and confirm `channelNumber` matches the target.

**Typical total**: 1 press to enter TV Plus + digit keys for channel number = **~4-5 key presses**.

### Switch between category tabs

**From anywhere in a tab's content area:**

1. Press `up` repeatedly until focus reaches the top controls. Alternatively, scroll to the bottom 72x72 button and press `enter` to jump directly to Category.
2. Run `dump-tree` or `get-focused` to check which tab is `[0] >>>` (추천=102, 라이브=108, Apps=114).
3. Calculate the exact number of `right` / `left` presses (추천 → 라이브 → Apps). If target is already focused, skip to step 5.
4. Press `right` / `left` exactly N times — do not overshoot (escapes to PrivacyChoice or Setting).
5. Press `enter` to activate.
6. Do **not** press `down` — focus auto-moves to the first content row.

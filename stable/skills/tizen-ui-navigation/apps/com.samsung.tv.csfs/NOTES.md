# AI Navigation Notes — com.samsung.tv.csfs

## UI Structure
<!-- Max 60 lines. Overview of the app's screen layout and widget hierarchy. -->
<!-- Replace entirely when updating — do not append. -->

The root tab container (`[Control:tab]`) holds all content for the active category tab. Its children change when switching between Recommendation, Live, and Applications. Row labels and contents are dynamic and may change when the home application's content is updated by Samsung.

**Recommendation tab** structure (top to bottom):
- First row: "지금 뜨는 콘텐츠" (1920x328) — pinned `삼성 TV 플러스` card + carousel (~21 items)
- Content/theme rows (1920x352 each): genre rows (코미디, 액션 스릴러, 다큐멘터리, 드라마, 전쟁), curated collections (대담한 탐험, 미래 비전 탐구, 환상적인 여정), live channel rows, 무료 영화, TVPlus 음악
- Special rows: BubbleTab (1920x256, inline category selector), Genre Bookmark (1920x256), 한국 Top 10 영화 (1920x424, taller ~568x320 tiles)
- Sponsored Row: ad tiles (1920x352)
- 미디어 옵션 (1920x288): settings/preferences row
- Launcher row (1920x112): small app shortcut tiles (112x112)
- Bottom button (72x72): `s/terra.bottom@d/media.category` — jump to top

**Live tab** structure (top to bottom):
- Banner (1920x528): Samsung TV Plus channel preview with "바로 보기" button
- "현재 방송 중" (1920x328): currently broadcasting, 456x256 tiles
- Content rows (1920x352 each): 바쁜 현대인 집중! 핫 클립 채널, 믿고 보는 장수 예능!, OnNow Sports, Samsung TV Plus : Live Events, and others
- EPG row (1920x296): 12 genre filter tiles (448x112), aggressively recycles element IDs
  - Genres: 실시간, 뉴스, 예능, 드라마, 어린이, 시사/교양, 라이프스타일, 기술/게임/과학, 스포츠, 쇼핑, 음악, 영화
- Bottom button (72x72): `s/terra.bottom@d/live.tab` — jump to top

**Applications tab** structure (top to bottom):
- Welcome banner (1920x360): non-focusable ("앱스에 오신 것을 환영합니다")
- RECOMMENDED row (240px): small icon tiles (112x112), divider between "삼성 제공" and "추천" sections
- EDITORSCHOICE row (496px): large app cards (416x368) with install status
- DOWNLOADED row (240px): small icon tiles (112x112)
- LIFESTYLEAPPSWW row (496px): large app cards (416x368)
- POPULAR_THIS_WEEK row (240px): small icon tiles (112x112)
- BESTAPPS row (496px): large app cards (416x368)
- GROUPCONTENT row (312px): group cards (456x152) — 음악 & 라디오, 심심풀이용 앱, 편리한 생활
- GENRE row (240px): genre cards (456x152) — 동영상, 스포츠, 게임, 라이프스타일, 정보, 교육
- SETTING row (336px): setting tiles (176x192) — 자동 업데이트, 모두 업데이트, 앱 설정
- Bottom button (72x72): `s/terra.bottom@d/apps.tab` — jump to top

Each list container contains a `[Control:items]` child with the navigable items.

The description label (elementId pattern `:1.XX:142`) shows the title of the currently focused item across ALL rows in the Recommendation tab — not just the first row. Use it to identify items in rows that have no inline text (e.g., "한국 Top 10 영화").

## Tips
<!-- Max 20 items. Practical shortcuts and navigation patterns. -->
<!-- Each tip is one line. Replace weak tips with better ones. -->

- Element IDs change when the UI reloads (e.g. `:1.19:xxx` becomes `:1.21:xxx`), so always re-dump the tree after any app transition or back navigation.
- Pressing ENTER on a focused content item in the Recommendation tab opens `com.samsung.tv.ContentsBrowserDetail`, which may route to third-party providers (Watcha, Netflix, etc.).
- Pressing ENTER on the Live tab banner launches Samsung TV Plus: `com.samsung.tv.csfs` → `org.tizen.tv-viewer` → `HEPsqFNie0.tvplusstandalone`.
- **Prefer the Live tab** for entering Samsung TV Plus when the goal is to watch a channel. The banner provides a reliable entry point, while the Recommendation tab's first row may change after content refreshes.
- **Always verify with `tizen-context-cli`** after pressing `enter` to launch any app. Confirm the expected `appId` is in the foreground before proceeding with further actions.
- Category tab order: 추천 → 라이브 → Apps. The focused tab is marked `[0] >>>` in `dump-tree` output.
- After activating a category tab with `enter`, focus auto-moves to the first focusable row — do NOT press `down`, and skip `get-focused` verification.
- The bottom button (72x72, `s/terra.bottom@d/*`) in every tab jumps focus back to the Category bar when pressed with `enter`.
- The EPG row in the Live tab has 12 genre categories with aggressive element ID recycling — re-dump after each scroll step.
- **DO NOT navigate the EDITORSCHOICE row** in the Apps tab. It aggressively recycles element IDs, making nav ordering unreliable. Use DOWNLOADED row to launch installed apps, or RECOMMENDED row for popular apps. The `dump-tree` output marks this row as `DO NOT NAVIGATE`.
- Large card rows (BESTAPPS, LIFESTYLEAPPSWW) also recycle element IDs — read the app name from `MainInfo` child instead of relying on IDs.
- Genre rows in the Recommendation tab are lazy-loaded — their items only populate when the row is scrolled into the visible area.
- The `삼성 TV 플러스` pinned card in the first Recommendation row always appears at `(0,0)` with a `focusable` flag regardless of actual focus — always check which item is marked `>>>` in `dump-tree`.
- DOWNLOADED row is 2 `down` presses from initial focus after entering the Applications tab — it contains all installed apps.
- **Always use DOWNLOADED to launch installed apps.** Store rows (EDITORSCHOICE, BESTAPPS, etc.) open the store detail page on `enter`, requiring a second `enter` on "실행" to launch. DOWNLOADED launches apps directly with one `enter`. Only use store rows when the app needs installation.
- Before navigating to DOWNLOADED, check if the target app appears in RECOMMENDED (0 down presses, initial focus) — popular apps like YouTube, Netflix, Spotify are there.
- When entering an unfocused row, focus starts at the leftmost item (if the row hasn't been scrolled in the current session). Activating a category tab resets all rows to initial positions. If a prior `dump-tree` showed the row's contents, you know the starting position — no need to dump again after entering the row.
- Batch key presses only when the path is known (e.g., "2 downs to DOWNLOADED"). When searching for content (unknown target), press down one at a time and check each row's label — do not batch during search.
- "한국 Top 10 영화" row items have no inline text in `dump-tree`, but the focused item's title appears in the description label (`:1.XX:142`). Use `dump-tree --id :1.XX:142` to read the title, then `left` / `right` to scan other items.
- Optimal path to launch an installed app: up to Category → right to Apps → enter → down ×2 (DOWNLOADED) → left/right to find app → enter (~7-10 key presses total).
- Row order in Applications tab by `down` press count from initial focus: RECOMMENDED (0) → EDITORSCHOICE (1) → DOWNLOADED (2) → LIFESTYLEAPPSWW (3) → POPULAR_THIS_WEEK (4) → BESTAPPS (5) → GROUPCONTENT (6) → GENRE (7) → SETTING (8).
- EDITORSCHOICE row app names are nested: item → ImageTile → MainInfo → text label (depth 4). Use `dump-tree --id <items-id>` to get the full content including names.
- After pressing Install on an app detail page, a banner dialog appears with "열기" (Open) button auto-focused — press enter immediately to launch the newly installed app.

## Known Issues
<!-- Max 10 items. Quirks, bugs, or unexpected behaviors encountered. -->
<!-- Remove resolved issues. -->

- Content carousel items shuffle dynamically — the same item may have different elementIds across sessions or UI reloads.
- Navigating to a distant item in a long row requires many key presses since only nearby items are clickable. There is no jump/search mechanism within a row.
- The EPG row's aggressive element ID recycling means the same container element shows different genre content as you scroll — cannot identify genre by element ID alone.
- Row labels and contents are dynamic and may change when the home application's content is updated by Samsung.
- **Xbox launch: `com.samsung.tv.quickpanel` is normal.** After pressing `enter` on the Xbox tile, `tizen-context-cli` may briefly return `com.samsung.tv.quickpanel` — this is the Xbox controller connection guide overlay, not an error. The actual appId (`GHI3a0zMSx.XboxGamePass`) confirms successful launch. Do not treat quickpanel as a failure when Xbox was the intended target.

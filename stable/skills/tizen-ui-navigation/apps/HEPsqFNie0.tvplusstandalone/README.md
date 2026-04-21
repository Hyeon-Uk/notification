# HEPsqFNie0.tvplusstandalone

Samsung TV Plus standalone application. A free, ad-supported streaming service providing live TV channels across genres including news, variety, drama, kids, sports, and music.

## General Rules

- **App type**: Web application — `tizen-aurum-cli dump-tree` does **not** work. UI element inspection is unavailable.
- **Context tool**: Use `tizen-context-cli` to read the current channel name, number, genre, program name, and timing information. This is the primary way to understand what is playing.
- **Screenshot**: Use `tizen-context-cli screenshot --capture` for visual confirmation. DRM-protected content (music videos, etc.) returns `drm_protected` error for content type; UI capture shows only the overlay layer.
- **Allowed operations**:
  - `tizen-aurum-cli key --xf86 XF86RaiseChannel` — channel up
  - `tizen-aurum-cli key --xf86 XF86LowerChannel` — channel down
  - `tizen-aurum-cli key --xf86 <digit>` (0–9) — direct channel number entry
  - `tizen-aurum-cli key back` — go back / exit TV Plus
  - `tizen-aurum-cli key enter` — confirm / interact with overlays
  - `tizen-context-cli` — read current broadcast info

  Standard directional keys (`up`, `down`, `left`, `right`) do **not** change channels in this app. Do **not** use `key <digit>` (without `--xf86`) — it sends the wrong keycode and may exit the app.

- **Timing**: Insert a 0.5-second delay between sequential channel up/down changes. When entering a channel number with digit keys, use a **0.3-second** delay between digits. When checking context after channel changes, wait **3 seconds** for the broadcast metadata to update.

## Entering Samsung TV Plus

### Direct launch (preferred)

Launch TV Plus directly with `wascmd`:

```bash
wascmd -r HEPsqFNie0.tvplusstandalone
```

Wait 3 seconds for the app to fully load. After launch, `tizen-context-cli` may return `DataValid: "0"` with empty broadcast fields. To get valid context, send one channel change (`XF86RaiseChannel` or `XF86LowerChannel`), then navigate back to the target channel. After that, wait 3 seconds and read context.

### From Home screen

1. From the **Recommendation** tab (default), the first content row has a pinned **삼성 TV 플러스** card that is typically the focused item.
2. Press `enter` to launch Samsung TV Plus.
3. The app transitions: `com.samsung.tv.csfs` → `org.tizen.tv-viewer` → `HEPsqFNie0.tvplusstandalone`.
4. Wait 3 seconds for the app to fully load.
5. Follow the same context initialization steps above.

**Alternative**: Navigate to the **라이브 (Live)** tab → press `enter` on the banner.

## Channel Navigation

### Using play-content tool (preferred)

Use `tizen-play-content-cli tvplus_channel <number>` to tune to a channel. It launches the TV Plus app and sends digit keys with proper delays automatically:

```bash
tizen-play-content-cli tvplus_channel 503
tizen-play-content-cli tvplus_channel 725
```

After the command completes, wait **3 seconds** before reading `tizen-context-cli` for the updated channel info.

### Manual digit entry (fallback)

If `tizen-play-content-cli` is unavailable, send each digit as an `--xf86` key with a **0.3-second** delay between digits:

```bash
tizen-aurum-cli key --xf86 5   # first digit
# wait 0.3s
tizen-aurum-cli key --xf86 0   # second digit
# wait 0.3s
tizen-aurum-cli key --xf86 2   # third digit → channel 502
```

This works for both 3-digit (e.g. 502, 725) and 4-digit (e.g. 1005) channel numbers. The delay between digits **must be 0.3 seconds** — longer delays (0.5s+) cause the TV to process digits separately instead of as a group.

After sending the digits, wait **3 seconds** before reading `tizen-context-cli` for the updated channel info.

### Sequential channel browsing

Use `XF86RaiseChannel` (channel up) and `XF86LowerChannel` (channel down) to browse channels one at a time. There is no channel guide overlay or genre filter within the app.

### Identifying the current channel

Run `tizen-context-cli` to get:

```json
{
  "broadcast": {
    "channelName": "SMTOWN",
    "channelNumber": "725",
    "genre": "음악",
    "programName": "2025 SMTOWN: THE CULTURE THE FUTURE 앨범",
    "source": "TVPLUS",
    ...
  }
}
```

Key fields:
| Field | Description |
|---|---|
| `channelName` | Channel name (may be Unicode-escaped in raw output) |
| `channelNumber` | Channel number as string |
| `genre` | Genre in Korean (음악, 예능, 드라마, 뉴스, etc.) |
| `programName` | Currently playing program title |
| `source` | Always `"TVPLUS"` when in Samsung TV Plus |

### Finding a specific channel

When you know the channel name or number, look it up in the channel list in `NOTES.md` and use direct channel entry with digit keys.

### Finding a specific genre

There is no jump-to-genre feature. To find a channel of a particular genre, look up the channel list in `NOTES.md` which is organized by genre, then use direct channel entry. As a fallback, use a scan loop:

1. Send `XF86RaiseChannel`.
2. Wait 3 seconds for the broadcast metadata to update.
3. Read `tizen-context-cli` and check the `genre` field.
4. Repeat until the target genre is found.

## Channel Map (Korea)

Observed channel ranges (approximate, may shift over time):

| Channel Range | Genre | Example Channels |
|---|---|---|
| ~500–510 | 뉴스 (News) | 502 연합뉴스TV |
| ~510–560 | 예능 (Variety) | 538 JTBC4, 553 맛있는 녀석들 |
| ~560–600 | 예능 (Variety) | 583 MBC 오은영리포트 결혼지옥 |
| ~600–650 | 드라마 (Drama) | 635 tvN 선재 업고 튀어 |
| ~700–750 | 음악 (Music) | 725 SMTOWN |

Music channels (genre: 음악) are located in the **700+ range**. From the default starting channel (~500), reaching a music channel requires approximately 150–200 channel-up presses.

## Screenshot Limitations

| Mode | What it captures | Music content |
|---|---|---|
| `auto` / `drm` | Video content (when not DRM-protected) | Blocked — "Capture is copy-protected" |
| `tbm` | UI overlay layer only | Shows only "TV Plus(Standalone)" text |

For music channels, rely on `tizen-context-cli` for program identification rather than screenshots.

## Exiting Samsung TV Plus

- Press `back` to return to the Home screen (`com.samsung.tv.csfs`).
- Press `home` to go directly to the Home screen.

---
name: tizen-content
description: Search and play content on Samsung TV. Use when the user wants to find, search, recommend, or play movies, TV shows, and YouTube videos. Orchestrates tizen-search-content-cli and tizen-play-content-cli.
argument-hint: <SEARCH QUERY>
user-invocable: true
allowed-tools: true
---

# Tizen Tool Content

Search and play movies, TV shows, and YouTube videos on Samsung TV. This skill orchestrates two tools:

- **`tizen-search-content-cli`** — searches the Samsung SearchAll API and YouTube
- **`tizen-play-content-cli`** — launches content on the TV via `wascmd`

## Intent-Based Workflow

Choose the workflow based on the user's intent:

| User says | Action |
|-----------|--------|
| "틀어줘", "켜줘", "재생해줘", "play", "보여줘" | Search → **play directly** via `tizen-play-content-cli` (no recommendation UI) |
| "찾아줘", "뭐 있어?", "추천해줘", "골라볼게" | Search → **show recommendation list** via `tizen-hud-cli show-reco --expand` |

```bash
# "틀어줘" → play directly
tizen-search-content-cli "Batman" --youtube --limit 1 | tizen-play-content-cli

# "찾아줘" → show recommendation list
tizen-search-content-cli "Batman" --youtube --limit 3 \
  | tizen-search-content-cli "Batman" --limit 3 \
  | tizen-hud-cli show-reco --expand
```

## Recommended Workflow (for recommendation list)

When a user asks about a topic (e.g., "I'm interested in Batman"), don't just do a single search. Build a rich set of results by combining multiple searches with pipes:

1. **Search YouTube** for reviews, interviews, or behind-the-scenes content related to the topic
2. **Search the main catalog** for the content itself (movies, shows)
3. **Search related content** — the actor's other works, upcoming sequels, similar titles
4. **Pipe all results together** and display them on TV via `tizen-hud-cli show-reco`
5. **Let the user browse and select** from the recommendation UI on the TV screen

Example — user says "I'm interested in Batman":
```bash
tizen-search-content-cli "Batman review" --youtube --limit 2 \
  | tizen-search-content-cli "Batman interview" --youtube --time year --limit 1 \
  | tizen-search-content-cli "Batman" --limit 3 \
  | tizen-search-content-cli "Robert Pattinson" --limit 2 \
  | tizen-hud-cli show-reco
```

This gives the user a curated mix: YouTube reviews, an interview clip, the Batman movies/shows, and related content from the lead actor — all displayed on the TV for browsing and selection.

## Query Translation for Non-YouTube Search

The Samsung SearchAll API is a structured catalog — it matches **exact content titles, actor names, and director names**, not natural language. YouTube search handles fuzzy/contextual queries natively, but non-YouTube search does not.

When the user gives a vague or contextual query (e.g., "요즘 핫한 영화", "SF 영화 추천해줘", "봄에 볼만한 드라마"), you **must translate** the query into one or more precise search terms before calling the non-YouTube search. Use your knowledge to come up with specific titles, actors, or directors that match the user's intent.

| User says | YouTube query (pass as-is) | Non-YouTube queries (translate) |
|-----------|---------------------------|-------------------------------|
| "봄에 볼만한 영화 추천해줘" | "봄 영화 추천" | "리틀 포레스트", "벚꽃 엔딩", "4월 이야기" |
| "요즘 핫한 SF 영화" | "2026 SF 영화 추천" | "듄", "인터스텔라", "아바타" |
| "톰 크루즈 나오는 영화" | "톰 크루즈 영화" | "톰 크루즈" (actor name — API handles this) |
| "배트맨" | "배트맨 리뷰" | "배트맨" (already precise — pass as-is) |
| "I want to watch Batman" | "Batman review" | "Batman" (already precise — pass as-is) |

**Rules:**
- **Match the user's language** — use search terms in the same language as the user's prompt. If the user speaks Korean, search with Korean titles (e.g., "인터스텔라" not "Interstellar"). If the user speaks English, use English titles. The `--lang` auto-detection adjusts the API's language setting based on the search text's script, so matching the language gives the best results.
- If the query is already a precise title or person name, pass it as-is to non-YouTube search.
- If the query is contextual/descriptive, break it into **multiple specific searches** for known titles, actors, or directors that fit the user's intent.
- Prefer well-known, likely-available content in the Korean market.
- Combine multiple translated searches via pipes to build a diverse result set.

Example — user says "요즘 볼만한 액션 영화 추천해줘" (Korean prompt → Korean search terms):
```bash
tizen-search-content-cli "액션 영화 추천" --youtube --limit 2 \
  | tizen-search-content-cli "존 윅" --limit 2 \
  | tizen-search-content-cli "미션 임파서블" --limit 2 \
  | tizen-search-content-cli "탑건" --limit 2 \
  | tizen-hud-cli show-reco --expand
```

Example — user says "recommend me some action movies" (English prompt → English search terms):
```bash
tizen-search-content-cli "action movie recommendations" --youtube --limit 2 \
  | tizen-search-content-cli "John Wick" --limit 2 \
  | tizen-search-content-cli "Mission Impossible" --limit 2 \
  | tizen-search-content-cli "Top Gun" --limit 2 \
  | tizen-hud-cli show-reco --expand
```

## When to Use

- User wants to **search** for movies, TV shows, or YouTube videos
- User wants to **play** content on their TV
- User asks to "find a movie about X" or "play X"
- User expresses interest in a topic and you want to build a recommendation set

## When NOT to Use

- User wants to **browse the TV UI** (use `tizen-ui-navigation` instead)
- User **selects content from the recommendation UI** — `tizen-hud-cli` launches deeplinks directly via `agent-hud-service`

---

## Search Tool Reference (`tizen-search-content-cli`)

```bash
tizen-search-content-cli <search_term>
tizen-search-content-cli --help
tizen-search-content-cli --api-key <key> <search_term>
tizen-search-content-cli --raw <search_term>
tizen-search-content-cli <search_term> --youtube
```

### Search Options

| Option | Description |
|---|---|
| `-h`, `--help` | Show help message |
| `--schema` | Print MCP-compatible JSON schema (JSON Schema Draft 7) and exit |
| `--api-key <key>` | Provide API key manually (overrides auto retrieval from buxton2) |
| `--raw` | Print raw server response without reformatting |
| `-l`, `--limit <N>` | Limit number of new search results (does not affect piped input) |
| `-p`, `--provider <name>` | Filter results by content provider name (case-insensitive) |
| `-g`, `--genre <name>` | Filter results by genre (case-insensitive) |
| `-L`, `--lang <locale>` | Language/region for SearchAll API (default: `auto`). `auto` detects from search text: Korean chars → `ko_KR`, Japanese → `ja_JP`, CJK ideographs → system locale, ASCII-only → `en_US`. Explicit values: `en_US`, `ko_KR`, `ja_JP`, etc. |
| `-y`, `--youtube` | Search YouTube directly via web scraping (no API key needed) |
| `-t`, `--time <filter>` | Time filter for `--youtube`: `hour`, `today`, `week`, `month`, `year`, `all` |

The API key is automatically retrieved from the device via buxton2 when available. Use `--api-key` to override or when buxton2 is not available.

YouTube mode (`--youtube`) does not require an API key — it fetches results by scraping YouTube search pages directly.

Exit codes: `0` success, `1` runtime error, `2` usage error (bad flags, missing args).

### Search Pipe Support

Reads JSON envelope (or legacy raw array) from stdin when piped and merges upstream results with new search results. Filters and limit apply only to newly searched results, not piped input. If upstream reports an error, it is propagated.

```bash
# Merge multiple searches (filters/limit apply only to new results, not piped input)
tizen-search-content-cli "Batman" --limit 3 | tizen-search-content-cli "Superman" --limit 2
```

### Search Output

All output is a JSON envelope on stdout. Use `--schema` to get the full machine-readable schema.

**Success (single search):**
```json
{
  "status": "success",
  "tool": "tizen-search-content-cli",
  "message": "5 total results from: \"Batman\"(5)",
  "data": {
    "queries": [
      {"query": "Batman", "source": "searchall", "result_count": 5}
    ],
    "result_count": 5,
    "results": [ ... ]
  }
}
```

**Success (piped multi-search):**
```json
{
  "status": "success",
  "tool": "tizen-search-content-cli",
  "message": "4 total results from: \"Batman review\"(2), \"Batman\"(2)",
  "data": {
    "queries": [
      {"query": "Batman review", "source": "youtube", "result_count": 2},
      {"query": "Batman", "source": "searchall", "result_count": 2}
    ],
    "result_count": 4,
    "results": [ ... ]
  }
}
```

**Error:**
```json
{
  "status": "error",
  "tool": "tizen-search-content-cli",
  "code": "empty_results",
  "message": "No results found for \"xyz\"",
  "data": null
}
```

Error codes: `missing_argument`, `invalid_argument`, `api_key_error`, `network_error`, `parse_error`, `pipe_parse_error`, `upstream_error`, `not_supported`.

Note: Empty results are **not** an error — the tool returns success with `result_count: 0` and an empty `results` array. This allows pipe chains to continue (e.g., a second search can add its own results even if the first found nothing). The final consumer (`tizen-hud-cli`, `tizen-play-content-cli`) decides if empty is a problem.

The `data.results` array contains content items. Each item is one of the following types:

**UMD content** (movies, shows):
```json
{
  "type": "umd_movie",
  "title": "Batman Begins",
  "year": "2005",
  "rating": "12+",
  "running_time": 8400,
  "genres": ["Action", "Adventure"],
  "providers": [
    {
      "name": "wavve",
      "app_id": "...",
      "icon_url": "...",
      "license": "rental",
      "quality": "FHD",
      "price": 1540,
      "currency": "KRW",
      "deeplink": "movie?movieid=MV_EN01_EN000000008"
    }
  ]
}
```

**YouTube content**:
```json
{
  "type": "youtube",
  "title": "Batman Review - Best Superhero Movie?",
  "poster_url": "https://i.ytimg.com/vi/.../hqdefault.jpg",
  "genres": [],
  "providers": [
    {
      "name": "YouTube",
      "app_id": "111299001912",
      "icon_url": "https://www.google.com/s2/favicons?sz=128&domain=youtube.com",
      "deeplink": "dQw4w9WgXcQ",
      "quality": "Channel Name"
    }
  ]
}
```

**VC content** (live/scheduled video):
```json
{
  "type": "vc",
  "title": "Program Title",
  "content_type": "movie",
  "genres": ["Drama"],
  "channel_name": "Channel Name",
  "channel_number": "123",
  "service_id": "...",
  "start_time": "2026-03-10T10:00:00",
  "end_time": "2026-03-10T12:00:00",
  "duration": "7200"
}
```

---

## Play Tool Reference (`tizen-play-content-cli`)

```bash
tizen-play-content-cli <provider> <content_id> [OPTIONS]
tizen-play-content-cli --help
```

### Providers

| Provider | Description | content_id |
|---|---|---|
| `youtube` | YouTube video | Video ID (e.g., `dQw4w9WgXcQ`) |
| `tving` | TVing content | Deeplink (e.g., `M000272935`) |
| `tvplus` | TV Plus (Samsung TV Plus) | Deeplink JSON |
| `apple_tv` | Apple TV content | Deeplink URL |
| `tvplus_channel` | TV Plus live channel | Channel number (e.g., `503`) |
| `deeplink` | Generic provider | Deeplink (requires `--app-id`) |

### Play Options

| Option | Description |
|---|---|
| `-h`, `--help` | Show help message |
| `--schema` | Print MCP-compatible JSON schema (JSON Schema Draft 7) and exit |
| `--app-id <id>` | App ID for generic `deeplink` provider |
| `-n`, `--dry-run` | Print the wascmd command without executing |

### App IDs

| Provider | App ID |
|---|---|
| YouTube | `111299001912` |
| TVing | `3201901017752` |
| TV Plus | `3202006020992` |
| Apple TV | `3201807016597` |
| TV Plus Channel | `3202405035480` |

Exit codes: `0` success, `1` runtime error, `2` usage error (bad flags, missing args). All output is a JSON envelope on stdout. Use `--schema` for the full machine-readable schema.

### Play Output

**Success (pipe mode):**
```json
{
  "status": "success",
  "tool": "tizen-play-content-cli",
  "message": "Playing: Batman Begins via YouTube",
  "data": {
    "action": "play",
    "title": "Batman Begins",
    "provider": "YouTube",
    "app_id": "111299001912",
    "content_type": "youtube"
  }
}
```

**Success (direct mode):**
```json
{
  "status": "success",
  "tool": "tizen-play-content-cli",
  "message": "Playing: dQw4w9WgXcQ via youtube",
  "data": {
    "action": "play",
    "provider": "youtube",
    "content_id": "dQw4w9WgXcQ",
    "app_id": "111299001912"
  }
}
```

**Error:**
```json
{
  "status": "error",
  "tool": "tizen-play-content-cli",
  "code": "no_playable_content",
  "message": "No playable content found in pipe input",
  "data": null
}
```

Error codes: `missing_argument`, `invalid_argument`, `no_playable_content`, `empty_pipe`, `pipe_parse_error`, `upstream_error`, `exec_error`, `play_error`, `not_supported`.

### Play Pipe Support

If `content_id` is omitted, reads a JSON envelope (or legacy raw array) from stdin and automatically selects content to play. If upstream reports an error, it is propagated.

**Provider selection logic:** Scans all items and all providers to find a preferred provider (YouTube, TVing, Apple TV — in that order). If no preferred provider is found, falls back to the first playable item using the generic deeplink format. This preference exists because deeplink launching has only been verified with these providers so far.

---

## Quick Reference Examples

```bash
# Search with filters
tizen-search-content-cli "Batman" --limit 3 --provider wavve
tizen-search-content-cli "Batman" --genre "Action"
tizen-search-content-cli "tech news" --youtube --time today --limit 3

# Play by provider (see Providers table above for content_id formats)
tizen-play-content-cli tvplus '{"deeplink_data":{"content_type":"episodic","content_id":"TZU_Istanbul_S10_EP08_FHD_KR"}}'
tizen-play-content-cli apple_tv 'https://tv.apple.com/kr/episode/chapter-sixteen/umc.cmc.2xlatjdgqcadgesg6nhqmteu3'
tizen-play-content-cli tvplus_channel 503
tizen-play-content-cli deeplink 'movie?id=123' --app-id 1234567890
```

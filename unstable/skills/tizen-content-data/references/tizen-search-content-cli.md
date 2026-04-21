# tizen-search-content-cli
**Description**: Search movies, TV shows, and YouTube videos using Samsung SearchAll API or YouTube web scraping.

## Usage
```
tizen-search-content-cli [OPTIONS] <search_term>
```

## Arguments
| Argument | Required | Description |
|----------|----------|-------------|
| `<search_term>` | Yes | Search query string |

## Options
| Option | Description |
|---|---|
| `--api-key <key>` | Provide API key manually (overrides buxton2 auto retrieval) |
| `--raw` | Print raw server response |
| `-l, --limit <N>` | Limit number of new search results |
| `-p, --provider <name>` | Filter by content provider (case-insensitive) |
| `-g, --genre <name>` | Filter by genre (case-insensitive) |
| `-L, --lang <locale>` | Language/region (default: auto-detect from search text) |
| `-y, --youtube` | Search YouTube via web scraping (no API key needed) |
| `-t, --time <filter>` | Time filter for YouTube: hour, today, week, month, year, all |

## Pipe Support
Reads JSON envelope from stdin and merges upstream results with new search results. Filters and limit apply only to new results.


## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-search-content-cli`

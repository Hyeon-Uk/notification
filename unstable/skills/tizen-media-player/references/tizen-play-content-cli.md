# tizen-play-content-cli
**Description**: Play content on Tizen TV by provider and content ID using wascmd deeplinks.

## Usage
```
tizen-play-content-cli <provider> <content_id> [OPTIONS]
```

## Providers
| Provider | content_id | App ID |
|---|---|---|
| `youtube` | Video ID (e.g., `dQw4w9WgXcQ`) | 111299001912 |
| `tving` | Deeplink (e.g., `M000272935`) | 3201901017752 |
| `tvplus` | Deeplink JSON | 3202006020992 |
| `apple_tv` | Deeplink URL | 3201807016597 |
| `tvplus_channel` | Channel number (e.g., `503`) | 3202405035480 |
| `deeplink` | Generic deeplink (requires `--app-id`) | user-specified |

## Options
| Option | Description |
|---|---|
| `--app-id <id>` | App ID for generic `deeplink` provider |
| `-n, --dry-run` | Print the wascmd command without executing |

## Pipe Support
If `content_id` is omitted, reads JSON envelope from stdin and auto-selects content. Preferred providers: YouTube > TVing > Apple TV.


## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-play-content-cli`

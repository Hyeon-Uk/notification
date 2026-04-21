# tizen-hud-cli
**Description**: Control the agent HUD overlay on Tizen TV: show/hide text overlays and content recommendations.
**Depends on**: `org.tizen.agent-hud-service` ([github.sec.samsung.net/NextSolution/agent-hud-service](https://github.sec.samsung.net/NextSolution/agent-hud-service)) — HUD service must be running on the device.

## Subcommands
| Subcommand | Options | Description |
|---|---|---|
| `status` | | Show current HUD state (JSON) |
| `select` | `<uri>` | Select content item by URI |
| `show-reco` | `[--items <json>] [-e\|--expand]` | Show content recommendation list |
| `hide-reco` | | Hide content recommendation |
| `show-overlay` | `--text <text>` | Show text overlay on screen |
| `hide-overlay` | | Hide text overlay |

## Pipe Support
Reads search results from stdin (JSON envelope from `tizen-search-content-cli`) for `show-reco`.


## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-hud-cli`

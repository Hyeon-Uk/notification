# tizen-context-cli
**Description**: Query app/broadcast context, screenshots, and key events from agent-context-service via gRPC.
**Depends on**: `org.tizen.agent-context-service` ([github.sec.samsung.net/NextSolution/agent-context-service](https://github.sec.samsung.net/NextSolution/agent-context-service)) — context service must be running on the device.

## Subcommands
| Subcommand | Options | Description |
|---|---|---|
| `context` | | Query current app and broadcast context |
| `screenshot` | `[--capture] [--at <DURATION>] [-l\|--list] [-c\|--count N] [-w\|--window DUR] [-t\|--type ui\|content] [--save-to DIR]` | Get, capture, or list screenshots |
| `keys` | `[-c\|--count N]` | Get recent key event history |

## Global Options
| Option | Description |
|---|---|
| `--address <addr>` | agent-context-service address (default: localhost:50064) |
| `--schema` | Print MCP-compatible JSON schema and exit |


## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-context-cli`

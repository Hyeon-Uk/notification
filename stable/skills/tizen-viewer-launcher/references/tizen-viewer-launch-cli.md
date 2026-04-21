# tizen-viewer-launch-cli
**Description**: Launch org.tizen.tizen-tool-viewer with presentation JSON via Tizen App Control.

## Usage
```
cat presentation.json | tizen-viewer-launch-cli
tizen-viewer-launch-cli --file <path>
```

## Options
| Option | Description |
|---|---|
| `--file <path>` | Use an existing presentation JSON file instead of stdin |
| `--app-id <id>` | Target application ID (default: org.tizen.tizen-tool-viewer) |
| `--dry-run` | Read the payload but do not send the launch request |
| `--format json\|pretty` | Output format (default: json) |

## Pipe Support
Reads presentation JSON from stdin when `--file` is not provided.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-viewer-launch-cli`

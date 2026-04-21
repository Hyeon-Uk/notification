# tizen-presentation-validate-cli
**Description**: Validate presentation JSON files against the Samsung Tizen TV viewer schema.

## Usage
```
tizen-presentation-validate-cli <file>
tizen-presentation-validate-cli --stdin
tizen-presentation-validate-cli describe
```

## Arguments
| Argument | Required | Description |
|----------|----------|-------------|
| `<file>` | Yes (or `--stdin`) | Path to presentation JSON file |

## Options
| Option | Description |
|---|---|
| `--stdin` | Read presentation JSON from stdin |
| `--format json\|pretty` | Output format (default: pretty) |

## Subcommands
| Subcommand | Description |
|---|---|
| `describe` | Show validation rules |




## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-presentation-validate-cli`

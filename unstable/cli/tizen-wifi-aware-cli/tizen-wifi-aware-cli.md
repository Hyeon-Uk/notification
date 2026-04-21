# TIZEN-WIFI-AWARE-CLI(1)

## NAME
**tizen-wifi-aware-cli** - exercise public Tizen Wi-Fi Aware APIs from the command line

## SYNOPSIS
**tizen-wifi-aware-cli** <command> [arguments]

## DESCRIPTION
**tizen-wifi-aware-cli** wraps a small, finite subset of the public
`wifi-aware.h` API for smoke-testing initialization and publish or subscribe
configuration objects.

The tool only documents commands that are implemented in code. Commands return
immediately after the underlying public API call sequence finishes.

## COMMANDS REFERENCE

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `status` | none | Run an initialize/deinitialize roundtrip. |
| `session-types` | none | List public Wi-Fi Aware session types. |
| `publish-types` | none | List public Wi-Fi Aware publish types. |
| `subscribe-types` | none | List public Wi-Fi Aware subscribe types. |
| `publish-template` | `<service-name> [publish-type] [ttl] [match-filter] [info]` | Create, configure, and destroy a publish handle. |
| `subscribe-template` | `<service-name> [subscribe-type] [ttl] [min-distance] [max-distance] [match-filter] [info]` | Create, configure, and destroy a subscribe handle. |
| `help`, `--help`, `-h` | none | Show usage help. |
| `version`, `--version`, `-V` | none | Show tool version. |

## EXIT STATUS
`0` for success, `1` for invalid input or native API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-wifi-aware-cli`

## AUTHOR
Part of the **Tizen Tools** project.

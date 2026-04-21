# TIZEN-HTTP-CLI(1)

## NAME
**tizen-http-cli** - Legacy wrapper for the deprecated Tizen HTTP C API.

## SYNOPSIS
**tizen-http-cli** *COMMAND* [*ARGS*]  
**tizen-http-cli** **-h** | **--help** | **help**  
**tizen-http-cli** **-v** | **--version** | **version**

## DESCRIPTION
**tizen-http-cli** currently wraps `http.h`, whose public API surface is marked
`Deprecated since 9.0` in the local Tizen headers. For that reason, this tool
is intentionally kept as a minimal legacy compatibility wrapper and is excluded
from the non-deprecated enhancement batch in this pass.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **get** | `<url>` | Submits a legacy asynchronous HTTP GET transaction and reports whether submission succeeded. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or deprecated HTTP API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-http-cli`

## AUTHOR
Part of the **Tizen Tools** project.

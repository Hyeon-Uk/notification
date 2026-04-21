# `tizen-bundle-cli`

`tizen-bundle-cli` is a command-line frontend for the Tizen `bundle`
library. It builds, inspects, mutates, and re-encodes native Tizen
bundle payloads using real `libbundle` FFI bindings.

## Synopsis

`tizen-bundle-cli <COMMAND> [OPTIONS]`

## Description

The CLI emits and accepts a hex string representation of the encoded
`bundle_raw` bytes returned by `bundle_encode()`. This keeps the payload
shell-safe while still using the real native bundle serializer.

Supported value families:

- string
- string array
- byte
- byte array

## Commands

| Command | Description |
| --- | --- |
| `encode [key=value]...` | Create a bundle with string entries and print the encoded bundle as hex. |
| `encode-csv [key=v1,v2,v3]...` | Create a bundle with string-array entries parsed from CSV lists. |
| `encode-bytes [key=hex]...` | Create a bundle with byte entries from hex payloads. |
| `encode-byte-array [key=h1,h2,h3]...` | Create a bundle with byte-array entries from comma-separated hex payloads. |
| `decode <hex_bundle>` | Decode a bundle hex string and print every key/value pair with its type. |
| `count <hex_bundle>` | Print the number of entries inside a decoded bundle. |
| `type <hex_bundle> <key>` | Print the bundle type for one key. |
| `get <hex_bundle> <key>` | Print one decoded entry by key. |
| `delete <hex_bundle> <key>` | Remove a key from a decoded bundle and print the re-encoded bundle hex. |
| `duplicate <hex_bundle>` | Duplicate a decoded bundle with `bundle_dup()` and print the re-encoded copy. |
| `--version` | Print the tool version. |
| `--help` | Print command help. |

## Notes

- Hex payloads must contain an even number of hexadecimal characters.
- `encode-byte-array` expects each array element as one hex item in a
  comma-separated list, for example:
  `blobs=616161,626262,636363`
- `decode` and `get` print `String`, `StringArray`, `Byte`, and
  `ByteArray` entries only. Unknown bundle types are reported as
  failures.


## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-bundle-cli`

# TIZEN-YACA-CLI(1)

## NAME
**tizen-yaca-cli** - run finite public YACA digest, key, MAC, and symmetric-crypto queries on Tizen

## SYNOPSIS
**tizen-yaca-cli** *command*

## DESCRIPTION
**tizen-yaca-cli** is a one-shot command-line interface for the public Yet
Another Crypto API (YACA).

The tool initializes YACA, performs a single finite operation, prints the
result, and then cleans up the library state. Symmetric encryption commands use
a PBKDF2-derived AES-256 key and ECB mode so that the `encrypt` and `decrypt`
commands are deterministic and can roundtrip in shell-driven tests.

## COMMANDS REFERENCE

| Command | Description |
| --- | --- |
| **digest** `<MESSAGE>` | Calculate the SHA-256 digest of a message. |
| **encrypt** `<PASSWORD>` `<PLAINTEXT>` | Encrypt plaintext with a PBKDF2-derived AES-256 key in ECB mode and print the ciphertext as hex. |
| **decrypt** `<PASSWORD>` `<HEX_CIPHERTEXT>` | Decrypt a ciphertext produced by **encrypt** and print the plaintext. |
| **hmac** `<PASSWORD>` `<MESSAGE>` | Calculate an HMAC-SHA256 value using a PBKDF2-derived key. |
| **cmac** `<PASSWORD>` `<MESSAGE>` | Calculate an AES CMAC value using a PBKDF2-derived key. |
| **self-roundtrip** `<PASSWORD>` `<PLAINTEXT>` | Encrypt and decrypt a message in one call and print both the ciphertext and recovered plaintext. |
| **key-info** `--type <symmetric\|des\|iv> --bits <BITS>` | Generate a key and print its resolved YACA key type and bit length. |
| **key-export-import** `--type <symmetric\|des\|iv> --bits <BITS> --format <raw\|base64>` | Export a generated key and import it back, then print the imported metadata. |
| **help**, **--help**, **-h** | Show command help. |
| **version**, **--version**, **-V** | Show the tool version. |

## EXIT STATUS
**0**: Success.

**1**: Invalid arguments or runtime failure from the YACA API.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-yaca-cli`

## AUTHOR
Part of the Tizen Tools project.

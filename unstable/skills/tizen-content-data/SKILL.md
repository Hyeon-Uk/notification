---
name: tizen-content-data
description: Manages content and data operations including storage, MIME types, and content search. Use when working with file storage, MIME type operations, or content database search.
---

# Tizen Content Data

Manages content and data operations including storage, MIME types, and search.

## Quick reference

**LibStorage64**: Storage operations → `tizen-libstorage64-cli`
**MIME Type**: MIME type operations → `tizen-mime-type-cli`
**Search Content**: Content search → `tizen-search-content-cli`

## LibStorage64

Manage file storage (64-bit):

```bash
# Get storage info
tizen-libstorage64-cli info --path /opt/usr/media

# Get available space
tizen-libstorage64-cli get-free-space --path /opt/usr/media

# Get used space
tizen-libstorage64-cli get-used-space --path /opt/usr/media
```

See [tizen-libstorage64-cli.md](references/tizen-libstorage64-cli.md) for complete documentation.

## MIME Type

Manage MIME type operations:

```bash
# Get MIME type for file
tizen-mime-type-cli get --file /opt/media/document.pdf

# List all MIME types
tizen-mime-type-cli list

# Validate MIME type
tizen-mime-type-cli validate --type application/pdf
```

See [tizen-mime-type-cli.md](references/tizen-mime-type-cli.md) for complete documentation.

## Search Content

Search content database:

```bash
# Search by keyword
tizen-search-content-cli search --keyword music

# Search by type
tizen-search-content-cli search --type video

# Search by date
tizen-search-content-cli search --date 2024-01-01
```

See [tizen-search-content-cli.md](references/tizen-search-content-cli.md) for complete documentation.

## Common use cases

- **File manager**: Query storage space and file information
- **Content search**: Search content databases
- **MIME operations**: Determine file types and validate MIME types

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
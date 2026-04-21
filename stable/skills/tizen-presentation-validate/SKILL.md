---
name: tizen-presentation-validate
description: Validates presentation files and metadata. Use when validating presentation integrity, checking presentation metadata, or verifying presentation files.
---

# Tizen Presentation Validate

Validates presentation files and metadata.

## Quick reference

**Presentation Validate**: Presentation validation → `tizen-presentation-validate-cli`

## Presentation Validate

Validate presentation files:

```bash
# Validate presentation
tizen-presentation-validate-cli validate --file /path/to/presentation.pptx

# Get presentation metadata
tizen-presentation-validate-cli metadata --file /path/to/presentation.pptx

# Check presentation integrity
tizen-presentation-validate-cli check-integrity --file /path/to/presentation.pptx
```

See [tizen-presentation-validate-cli.md](references/tizen-presentation-validate-cli.md) for complete documentation.

## Common use cases

- **Presentation applications**: Validate presentation files
- **Content validation**: Check presentation integrity
- **Metadata verification**: Verify presentation metadata

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
# NAME

tizen_id - Tizen system identity generator

# SYNOPSIS

```bash
tizen_id
```

# DESCRIPTION

`tizen_id` is a Tizen system identity generator. It validates or initializes the `TIZEN_ID_PATH` local registry key using UUIDs bounded by standard base64 structures containing custom salt prefixes (`TzID`) and suffixes (`R6SUQ=`). This tool is typically executed during system boot to ensure the device has a valid unique identifier.

# OPTIONS

This tool does not accept any command-line options.

# OUTPUT FORMAT

- **Format:** Plain Text (silent on success)
- **Structure Details:** Typically operates silently on success. Logs identity creation diagnostics to stderr upon filesystem errors.

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Operation completed successfully |
| nonzero | Error - Execution failed (filesystem errors, permission denied) |

# FILES

- `/opt/etc/tizenid` - Tizen ID storage location

# EXAMPLES

## Generate or validate system identity

```bash
$ tizen_id
```

# SEE ALSO

uuidgen(1)

# PACKAGE

tizen-id
# NAME

appmgr_tool - Tizen Application Manager testing utility

# SYNOPSIS

```bash
appmgr_tool <APP_ID>
```

# DESCRIPTION

`appmgr_tool` is a testing utility for the Tizen Application Manager. It accepts an application ID as an argument and performs application management operations such as querying app information. Primarily intended for internal testing and debugging.

# OPTIONS

This tool does not accept any command-line options. It takes a single positional argument.

# ARGUMENTS

| Argument | Description |
|----------|-------------|
| `<APP_ID>` | Tizen application ID to query |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Outputs application information or error messages if the app is not found

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Application information retrieved |
| 1 | Error - Application ID not found or invalid argument |

# ENVIRONMENT

- Communicates with the application manager daemon via D-Bus
- Queries the application database for registered app IDs

# EXAMPLES

## Query application information

```bash
$ appmgr_tool org.tizen.setting
```

# SEE ALSO

app_launcher(1), pkgcmd(1)

# PACKAGE

app-manager
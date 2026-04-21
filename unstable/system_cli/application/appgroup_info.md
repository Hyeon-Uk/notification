# NAME

appgroup_info - Tizen application group information display tool

# SYNOPSIS

```bash
appgroup_info
```

# DESCRIPTION

`appgroup_info` displays the current application group information managed by the AUL framework. It shows the hierarchical relationships between grouped applications and the idle process count. This tool is used for debugging app grouping behavior.

# OPTIONS

This tool does not accept any command-line options.

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Outputs app group tree structure with headers and an idle count

**Example output (no active groups):**
```
### App Group Information ###

No app group
< Idle count : 0 >
### end ###
```

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - App group information printed |
| 1 | Error - AUL daemon communication failure |

# ENVIRONMENT

- Queries the AUL application group manager via D-Bus

# EXAMPLES

## View current app groups

```bash
$ appgroup_info
```

# SEE ALSO

app_launcher(1), aulctl(1)

# PACKAGE

aul
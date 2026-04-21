# NAME

appid2pid - Convert Tizen application ID to process ID

# SYNOPSIS

```bash
appid2pid <APP_ID>
```

# DESCRIPTION

`appid2pid` translates a Tizen application ID into its running process ID (PID). If the application is not running, it returns `-1`. This is a lightweight query tool for scripting and debugging purposes.

# OPTIONS

This tool does not accept any command-line options. It takes a single positional argument.

# ARGUMENTS

| Argument | Description |
|----------|-------------|
| `<APP_ID>` | Tizen application ID to query |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:** Outputs a single integer - the PID of the running application, or `-1` if not running

**Example output:**
```
1234
```

Or if not running:
```
-1
```

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Query completed (PID found or -1 returned) |
| 1 | Error - Invalid application ID or AUL daemon unreachable |

# ENVIRONMENT

- Queries the AUL daemon for running application status

# EXAMPLES

## Get PID of a running application

```bash
$ appid2pid org.tizen.setting
1234
```

## Check non-running application

```bash
$ appid2pid org.tizen.nonexistent
-1
```

## Use in shell script

```bash
#!/bin/bash
PID=$(appid2pid org.tizen.browser)
if [ "$PID" != "-1" ]; then
    echo "Browser is running with PID: $PID"
fi
```

# SEE ALSO

app_launcher(1), pidof(1), ps(1)

# PACKAGE

aul
# NAME

aulctl - Tizen application boot sequence management tool

# SYNOPSIS

```bash
aulctl <command> [arguments]
```

# DESCRIPTION

`aulctl` is Tizen's application boot sequence management tool. It manages the startup ordering, dependencies, and lifecycle of applications that are configured to launch during the system boot process. Supports listing, starting, stopping, and checking the status of boot sequence applications.

# COMMANDS

| Command | Arguments | Description |
|---------|-----------|-------------|
| `list-apps` | `[uid]` | Print the list of boot sequence apps (optional UID filter) |
| `is-active` | `<appid>` | Print whether the app is activated (`active` or `inactive`) |
| `status` | `<appid>` | Print the status of a boot sequence app |
| `list-dependencies` | `[uid]` | Print dependencies of boot sequence apps |
| `reload` | `[uid]` | Reload boot sequence app information from configuration |
| `start` | `<appid>` | Start the app according to its configured dependencies |
| `stop` | `<appid>` | Stop the app |

# OUTPUT FORMAT

- **Format:** Plain Text (stdout)
- **Structure Details:**
  - `list-apps`: Outputs app IDs
  - `is-active`: Outputs `active` or `inactive`
  - `status`: Outputs lifecycle state

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Command executed successfully |
| 1 | Error - Invalid command, app not found, or daemon communication failure |

# ENVIRONMENT

- Boot sequence configuration is managed by the AUL framework
- Application dependency graphs determine startup ordering

# EXAMPLES

## List boot sequence apps

```bash
$ aulctl list-apps
```

## List boot sequence apps for specific user

```bash
$ aulctl list-apps 5000
```

## Check if an app is active

```bash
$ aulctl is-active org.tizen.setting
```

## Get app status

```bash
$ aulctl status org.tizen.myservice
```

## Start an app with dependency resolution

```bash
$ aulctl start org.tizen.myservice
```

## Stop an app

```bash
$ aulctl stop org.tizen.myservice
```

## Reload boot sequence configuration

```bash
$ aulctl reload
```

# SEE ALSO

app_launcher(1), aul_test(1), systemd(1)

# PACKAGE

aul
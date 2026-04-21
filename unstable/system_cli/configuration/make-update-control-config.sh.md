# make-update-control-config.sh - Generate Tizen device update configuration

## NAME

**make-update-control-config.sh** - Generate configuration headers for Tizen device updates.

## SYNOPSIS

```bash
make-update-control-config.sh <delta_type> <url_logs> <server_url> <user_id> <api_key>
```

## DESCRIPTION

**make-update-control-config.sh** generates the configuration headers required for Tizen device updates. It configures the firmware update control parameters including differential payload settings and server authentication.

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `delta_type` | Differential payload format type. |
| `url_logs` | HTTP endpoint for previous success traces. |
| `server_url` | Firmware deployment routing endpoint. |
| `user_id` | Administrative user identifier string. |
| `api_key` | Authentication token for server access. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Operation executed successfully. |
| `1` | Error. Parameter invalid, permission denied, or process failure. |

## EXAMPLES

### Generate update configuration

```bash
$ make-update-control-config.sh delta http://logs.server.com http://update.server.com admin abc123
```

## NOTES

- Interfaces with Tizen framework paths such as `/opt/dbspace/` and `/opt/usr/apps/`.
- Logs failures to dlog (Tizen logging system).
- Requires proper authentication credentials.
- Used during Tizen system update configuration.

## SEE ALSO

**dlogutil**(1)

## PACKAGE

`tizen-update-control`
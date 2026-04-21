# TIZEN-PUSH-CLI(1)

## NAME
**tizen-push-cli** - Exercise public Tizen Push Service APIs from a
non-interactive shell.

## SYNOPSIS
**tizen-push-cli** *COMMAND* [*OPTIONS*]  
**tizen-push-cli** **-h** | **--help** | **help**  
**tizen-push-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-push-cli** provides command-line access to the public Push Service
APIs from `push-service.h`.

The tool focuses on finite lifecycle and query commands that are practical in
headless or shell-driven environments:
- connecting to the push daemon
- registering and deregistering an app id
- retrieving the current registration ID
- requesting unread notifications
- explicitly disconnecting a connection handle

The CLI uses a short GLib main loop to observe asynchronous state and result
callbacks without leaving a long-running process behind.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **connect** | `--app-id <APP_ID>` | Connects to the push service with `push_service_connect()` and prints callback state details when available. |
| **register** | `--app-id <APP_ID>` | Connects and attempts registration using `push_service_register()`. |
| **deregister** | `--app-id <APP_ID>` | Connects and attempts deregistration using `push_service_deregister()`. |
| **get-registration-id** | `--app-id <APP_ID>` | Connects and prints the current registration ID using `push_service_get_registration_id()`. |
| **request-unread** | `--app-id <APP_ID>` | Connects and requests unread notifications using `push_service_request_unread_notification()`. |
| **disconnect** | `--app-id <APP_ID>` | Connects and explicitly releases the connection with `push_service_disconnect()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or Push Service API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-push-cli`

## AUTHOR
Part of the **Tizen Tools** project.

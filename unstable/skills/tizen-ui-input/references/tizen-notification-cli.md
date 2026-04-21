# TIZEN-NOTIFICATION-CLI(1)

## NAME
**tizen-notification-cli** - Post and inspect native Tizen notification
objects.

## SYNOPSIS
**tizen-notification-cli** *COMMAND* [*OPTIONS*]  
**tizen-notification-cli** **-h** | **--help** | **help**  
**tizen-notification-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-notification-cli** provides command-line access to the native
Notification APIs from `notification.h`, `notification_status.h`, and
`notification_setting.h`.

The tool supports:
- posting standard or ongoing notifications
- posting short status popup messages
- building a local notification template and reading back its resolved fields
- querying the current notification visibility class
- listing supported notification type values

Commands that post or update system-visible notifications can fail if the
notification service is unavailable or if the process lacks the
`http://tizen.org/privilege/notification` privilege.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **post** | `[--type <noti|ongoing>] <TITLE> <CONTENT>` | `http://tizen.org/privilege/notification` | Creates a notification, sets title/content/time, and posts it. |
| **show-template** | `[--type <noti|ongoing>] <TITLE> <CONTENT>` | Standard | Creates a notification handle locally and prints the resolved type, title, content, and timestamp without posting it. |
| **status-message** | `<MESSAGE>` | Platform dependent | Posts a transient status popup through `notification_status_message_post()`. |
| **visibility-class** | None | Standard | Prints the current notification visibility-class value from the notification setting handle. |
| **list-types** | None | Standard | Prints the supported `notification_type_e` values documented by the public API. |

## EXIT STATUS
**0**: Success, or a documented runtime fallback reported with `[WARN]`.  
**1**: Invalid arguments or an unrecoverable Notification API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-notification-cli`

## AUTHOR
Part of the **Tizen Tools** project.

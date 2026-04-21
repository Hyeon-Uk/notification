# TIZEN-APP-EVENT-CLI(1)

## NAME
**tizen-app-event-cli** - inspect public app-event names and publish user events.

## SYNOPSIS
**tizen-app-event-cli** *COMMAND* [*OPTIONS*]  
**tizen-app-event-cli** **-h** | **--help** | **help**  
**tizen-app-event-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-app-event-cli** provides command-line access to the public
Tizen event APIs from `app_event.h`.

The tool supports two finite workflows:
- printing curated system-event metadata derived from the public header
- publishing or retaining last-data for user events through
  `event_publish_app_event()`, `event_publish_trusted_app_event()`, and
  `event_keep_last_event_data()`

Optional event payloads are encoded as JSON-driven `bundle` objects
whose values are strings or string arrays.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list-system-events** | None | Standard | Prints curated system-event names, keys, and values from `app_event.h`. |
| **describe-system-event** | `--name <ALIAS_OR_EVENT>` | Standard | Prints the key and known values for one public system event. |
| **publish-user-event** | `--name <EVENT_NAME> [--data-json <JSON_OBJECT>]` | Standard | Publishes one user event through `event_publish_app_event()` with optional string or string-array payload fields. |
| **publish-trusted-user-event** | `--name <EVENT_NAME> [--data-json <JSON_OBJECT>]` | Platform policy dependent | Publishes one trusted user event through `event_publish_trusted_app_event()` with optional string or string-array payload fields. |
| **keep-last-event** | `--name <EVENT_NAME>` | Standard | Marks one user-event name as keeping the latest data through `event_keep_last_event_data()`. |

User-event names must follow the `event.{sender_appid}.{name}` format
required by `app_event.h`.

`--data-json` accepts a JSON object whose values are either strings or
string arrays, for example `{"sender":"tizen-tools","targets":["cli","test"]}`.
The schema is strict: the object must not be empty, keys must be
non-empty and unique, arrays must be non-empty, and array items must be
unique strings.

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or an app-event API call failed.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-app-event-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# launch_app - Tizen application launcher

## NAME

**launch_app** - Launch a Tizen application by its application ID.

## SYNOPSIS

```bash
launch_app <appid>
```

## DESCRIPTION

**launch_app** is a minimal internal wrapper designed to trigger a Tizen application directly into execution by its application ID. It delegates immediate execution through the AUL (Application Launcher) system.

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `appid` | Application ID to launch (e.g., `org.tizen.setting`) |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Application launch request was processed properly. |
| `1` | Error. Unable to locate or authorize the supplied application ID. |

## EXAMPLES

### Launch the Settings application

```bash
$ launch_app org.tizen.setting
```

### Launch the Web Browser

```bash
$ launch_app org.tizen.browser
```

## NOTES

- On success, the process immediately forks and detaches with no output.
- On failure, a basic error message is displayed (e.g., `... launch failed`).
- Execution is delegated to the loader and process orchestrator via AUL.

## SEE ALSO

**aulctl**(1), **app_launcher**(1), **launch_debug**(1)

## PACKAGE

`aul`
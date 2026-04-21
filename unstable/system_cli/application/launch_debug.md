# launch_debug - Tizen application debug launcher

## NAME

**launch_debug** - Launch a Tizen application in SDK debug mode with an attached debugger.

## SYNOPSIS

```bash
launch_debug <appid> __AUL_SDK__ <debugger> __DLP_ATTACH_ARG__ --attach,:[port],[pid]
```

## DESCRIPTION

**launch_debug** launches a Tizen application in SDK debug mode. It starts the application with a specified debugger (e.g., GDB, LLDB) attached, supporting remote debugging with configurable port and PID for attach operations.

The tool communicates with the AUL (Application Launcher) daemon for application launch. The debugger must be available on the target device.

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `appid` | Application ID to launch in debug mode |
| `__AUL_SDK__` | AUL SDK protocol delimiter |
| `<debugger>` | Debugger name to use (e.g., `GDB`, `LLDB`, `ATTACH`) |
| `__DLP_ATTACH_ARG__` | Debug attach argument delimiter |
| `--attach,:[port],[pid]` | Attach specification with port and PID (format: `:port,pid`) |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Debug session started. |
| `1` | Error. Application not found or debugger unavailable. |

## EXAMPLES

### Attach debugger to an application

```bash
$ launch_debug org.tizen.myapp __AUL_SDK__ ATTACH __DLP_ATTACH_ARG__ --attach,:10003,1234
```

### Launch with GDB debugger

```bash
$ launch_debug org.tizen.myapp __AUL_SDK__ GDB __DLP_ATTACH_ARG__ --attach,:10003,5678
```

## NOTES

- The debugger must be installed on the target device.
- Port and PID must be specified for attach operations.
- Uses AUL SDK protocol for communication with the launcher daemon.

## SEE ALSO

**launch_app**(1), **aulctl**(1), **app_launcher**(1)

## PACKAGE

`aul`
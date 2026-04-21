# TIZEN-APP-MANAGER-CLI(1)

## NAME
**tizen-app-manager-cli** - Query and manage Tizen application contexts.

## SYNOPSIS
**tizen-app-manager-cli** *COMMAND* [*OPTIONS*]  
**tizen-app-manager-cli** **-h** | **--help** | **help**  
**tizen-app-manager-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-app-manager-cli** provides command-line access to the native
Tizen Application Manager APIs from `app_manager.h`,
`app_manager_extension.h`, `app_info.h`, and `app_context.h`.

The tool supports:
- listing installed applications
- listing running application contexts
- resolving `pid -> app_id`
- retrieving per-app metadata
- retrieving shared data/resource/trusted paths
- resuming or terminating running applications
- streaming lifecycle and context events through the `ecore` main loop

Some commands require platform privileges or a running application
context.

## COMMANDS REFERENCE

| Command | Args | Privilege Requirement | Description |
| :--- | :--- | :---: | :--- |
| **list-installed** | None | Standard | Lists installed application IDs with label and type. |
| **list-contexts** | None | Standard | Lists running application contexts returned by `app_manager_foreach_app_context()`. |
| **list-running** | None | Standard | Lists running application contexts including sub apps. |
| **get-focused** | None | Platform | Prints the currently focused application ID. |
| **get-app-id** | `--pid <PID>` | Standard | Resolves an application ID from a process ID. |
| **get-info** | `--app-id <ID>` | Standard | Prints app ID, label, type, package, executable path, and icon path. |
| **get-shared-paths** | `--app-id <ID>` | Standard | Prints shared data, resource, and trusted paths for the app. |
| **is-running** | `--app-id <ID>` | Standard | Prints whether the application is running. |
| **resume** | `--app-id <ID>` | `appmanager.launch` | Requests the running application to resume. |
| **terminate** | `--app-id <ID>` | Platform `appmanager.kill` | Terminates a running application. |
| **terminate-bg** | `--app-id <ID>` | Platform dependent | Requests graceful background termination. |
| **terminate-no-restart** | `--app-id <ID>` | Platform `appmanager.kill` | Terminates a running app without restart handling. |
| **monitor** | None | Standard | Streams context and lifecycle events until interrupted. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, API failure, privilege failure, or unavailable
context.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-app-manager-cli`

## AUTHOR
Part of the **Tizen Tools** project.

# TIZEN-MMI-CLI(1)

## NAME
**tizen-mmi-cli** - Exercise public Tizen Multimodal Interaction (MMI)
workflow APIs from the shell.

## SYNOPSIS
**tizen-mmi-cli** *COMMAND* [*OPTIONS*]  
**tizen-mmi-cli** **-h** | **--help** | **help**  
**tizen-mmi-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-mmi-cli** provides command-line access to the public MMI APIs from
`mmi.h` and `mmi-workflow.h`.

The tool focuses on finite lifecycle operations that are practical in a
non-interactive shell:
- framework initialization checks
- standard workflow instance create, activate, and deactivate flows
- custom workflow prototype creation and instantiation
- workflow type assignment and query
- signal emission to a workflow instance

When the connected device does not support
`http://tizen.org/feature/multimodal_interaction`, runtime commands report
`[WARN]` and exit successfully so the capability gap is explicit without
turning device smoke tests into false failures.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **init** | None | Initializes and deinitializes the MMI framework to verify basic availability. |
| **create-standard** | `[--type <type>]` | Creates and destroys a standard workflow instance through `mmi_standard_workflow_instance_create()`. Default type is `voice-touch`. |
| **activate-standard** | `[--type <type>]` | Creates and activates a standard workflow instance through `mmi_workflow_instance_activate()`. |
| **deactivate-standard** | `[--type <type>]` | Creates, activates, and deactivates a standard workflow instance. |
| **create-custom** | `[--type <type>]` | Creates a workflow prototype with `mmi_workflow_create()`, sets its type, instantiates it with `mmi_custom_workflow_instance_create()`, and destroys it. |
| **workflow-type** | `[--type <type>]` | Creates a workflow prototype and prints the resolved type from `mmi_workflow_get_type()`. Without `--type`, the default prototype type is reported. |
| **signal** | `[--type <type>] [--name <name>]` | Creates a standard workflow instance and emits a signal with `mmi_workflow_instance_emit_signal()`. Default signal name is `TEST_SIGNAL`. |

Supported workflow types:
- `none`
- `voice-touch`

## EXIT STATUS
**0**: Success, or a documented `[WARN]` fallback on unsupported devices.  
**1**: Invalid arguments or MMI API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-mmi-cli`

## AUTHOR
Part of the **Tizen Tools** project.

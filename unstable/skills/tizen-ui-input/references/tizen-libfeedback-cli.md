# TIZEN-LIBFEEDBACK-CLI(1)

## NAME
**tizen-libfeedback-cli** - Play and inspect Tizen system feedback patterns.

## SYNOPSIS
**tizen-libfeedback-cli** *COMMAND* [*OPTIONS*]  
**tizen-libfeedback-cli** **-h** | **--help** | **help**  
**tizen-libfeedback-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-libfeedback-cli** provides command-line access to the native Tizen
Feedback APIs from `feedback.h` and `feedback-ids.h`.

The tool supports:
- listing public feedback types
- listing public feedback patterns
- playing a pattern with the default route or a specific type
- stopping currently playing vibration feedback
- checking whether a pattern is supported for a specific type

Pattern and type arguments can be provided as either numeric IDs or symbolic
names such as `tap`, `sound`, `FEEDBACK_PATTERN_TAP`, or
`FEEDBACK_TYPE_VIBRATION`.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **list-types** | None | Lists public `feedback_type_e` IDs and symbolic names. |
| **list-patterns** | None | Lists public `feedback_pattern_e` IDs and symbolic names. |
| **play** | `--pattern <ID\|NAME> [--type <ID\|NAME>]` | Plays a feedback pattern with the default route or the selected type. |
| **stop** | None | Stops currently playing vibration feedback. |
| **is-supported** | `--pattern <ID\|NAME> --type <ID\|NAME>` | Checks whether the selected pattern is supported for the selected type. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments or Feedback API failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-libfeedback-cli`

## AUTHOR
Part of the **Tizen Tools** project.

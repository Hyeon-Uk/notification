# tizen-aurum-cli
**Description**: UI automation via Aurum gRPC: send remote keys, click elements, inspect accessibility tree, navigate focus.
**Depends on**: `org.tizen.aurum-bootstrap` ([github.sec.samsung.net/tizen/aurum](https://github.sec.samsung.net/tizen/aurum)) — Aurum gRPC server must be running on the device.

## Subcommands
| Subcommand | Options | Description |
|---|---|---|
| `dump-tree` | `[--id ID] [--raw] [--focused]` | Dump UI widget tree (navigation-optimized by default) |
| `key` | `<name> [--action stroke\|press\|release] [--xf86 CODE]` | Send key event |
| `click` | `--id ID [--atspi]` or `--xy X Y` | Click element by ID or coordinates |
| `find` | `--text TEXT [--type TYPE] [--clickable] [--focused]` | Find elements by text/type |
| `get-focused` | | Get currently focused element |
| `get-attribute` | `--id ID --attr NAME` | Get element boolean attribute |
| `set-focus` | `--id ID` | Set keyboard focus to element |
| `move-to` | `--id ID` | Highlight element (visual focus) |
| `next` | `--id ID` | Get next focusable element |
| `prev` | `--id ID` | Get previous focusable element |
| `first` | `--id ID` | Get first focusable element |
| `last` | `--id ID` | Get last focusable element |
| `do-action` | `--id ID --action NAME` | Perform action on element |
| `screen-size` | | Get screen dimensions |
| `mouse-down` | `--xy X Y` | Mouse button down |
| `mouse-move` | `--xy X Y` | Mouse move |
| `mouse-up` | `--xy X Y` | Mouse button up |

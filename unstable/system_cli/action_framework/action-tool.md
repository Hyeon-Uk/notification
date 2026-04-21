# NAME

action-tool - Tizen Action Framework testing and management CLI

# SYNOPSIS

```bash
action-tool <command> [arguments]
```

# DESCRIPTION

`action-tool` is a testing and management CLI for Tizen's Action Framework. It allows executing, querying, listing, and subscribing to system actions defined as JSON-based descriptors. This tool is useful for testing action implementations and debugging the Action Framework.

# COMMANDS

| Command | Arguments | Description |
|---------|-----------|-------------|
| `execute` | `<json_string>` | Execute an action defined by a JSON descriptor |
| `get-action` | `<action_id>` | Get details of a specific action by ID |
| `list-actions` | - | List all available actions |
| `subscribe` | - | Subscribe to action events (streaming mode, blocks until interrupted) |

# OUTPUT FORMAT

- **Format:** Text / JSON (stdout)
- **Structure Details:**
  - `list-actions` outputs action IDs, one per line
  - `get-action` outputs JSON action descriptors
  - `subscribe` streams action events as they occur

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Command executed successfully |
| 1 | Error - Invalid JSON, action not found, or service not running |

# ENVIRONMENT

- Actions are defined in the Action Framework database
- Communicates via D-Bus with the action service daemon

# EXAMPLES

## List all available actions

```bash
$ action-tool list-actions
```

## Get details for a specific action

```bash
$ action-tool get-action my_action_id
```

## Execute an action with JSON descriptor

```bash
$ action-tool execute '{"action_id":"test_action","params":{}}'
```

## Subscribe to action events

```bash
$ action-tool subscribe
```

# SEE ALSO

action-db-recovery(1), actdctl(1)

# PACKAGE

action-frame
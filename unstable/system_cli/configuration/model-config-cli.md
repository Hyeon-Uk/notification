# model-config-cli - Tizen device model configuration tool

## NAME

**model-config-cli** - Read and write device model configuration values.

## SYNOPSIS

```bash
model-config-cli get <key>
model-config-cli set <key> <value>
```

## DESCRIPTION

**model-config-cli** reads and writes device model configuration values using the Tizen system-info key naming convention. It provides a simple get/set interface for device-type, manufacturer, and other model-specific properties.

## COMMANDS

| Command | Description |
|---------|-------------|
| `get <key>` | Get the value of a model configuration key. |
| `set <key> <value>` | Set the value of a model configuration key. |

## ARGUMENTS

| Argument | Description |
|----------|-------------|
| `key` | Configuration key in Tizen system-info namespace (e.g., `tizen.org/system/device_type`). |
| `value` | Value to set for the specified key. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Get or set operation completed. |
| `1` | Error. Key not found or invalid arguments. |

## EXAMPLES

### Get device type

```bash
$ model-config-cli get tizen.org/system/device_type
```

### Set device type

```bash
$ model-config-cli set tizen.org/system/device_type "Signage"
```

### Get manufacturer

```bash
$ model-config-cli get tizen.org/system/manufacturer
```

### Set manufacturer

```bash
$ model-config-cli set tizen.org/system/manufacturer "Samsung"
```

### Get model name

```bash
$ model-config-cli get tizen.org/system/model_name
```

## NOTES

- Uses the same key namespace as `system-info-tool` (`tizen.org/system/...`).
- `get` outputs the value string to stdout.
- Requires appropriate permissions to modify configuration values.

## SEE ALSO

**system-info-tool**(1), **vconftool**(1)

## PACKAGE

`tizen-system-info`
# NAME

at_spi2_tool - AT-SPI2 accessibility diagnostic and testing utility

# SYNOPSIS

```bash
at_spi2_tool [OPTIONS] [PARAMETER]
```

# DESCRIPTION

`at_spi2_tool` is a utility tool designed for the AT-SPI2 (Assistive Technology Service Provider Interface) core framework. It interacts with the accessibility D-Bus daemon to enumerate accessible applications, query their widget hierarchies, and evaluate states and actions, thereby serving as an automated testing and debugging utility for Tizen's UI accessibility features.

# OPTIONS

| Option | Long Option | Argument | Description |
|--------|-------------|----------|-------------|
| `-h` | `--help` | - | Show help message with usage information |
| `-v` | `--version` | - | Show actual version of tool (v1.1) |
| `-g` | `--show-legend` | - | Print AT-SPI state legend with state codes |
| `-l` | `--list-apps` | - | List all applications of desktop |
| `-a` | `--at-spi-client` | `<true\|false>` | Enable/disable org.a11y.Status.IsEnabled property |
| `-s` | `--sleep` | `<N>` | Sleep N seconds before operation |
| `-d` | `--tree-dump` | `<APP_NAME>` | Dump accessibility tree for selected application |
| `-c` | `--tree-check` | `<APP_NAME>` | Check accessibility tree integrity for selected application |
| `-f` | `--first-match` | - | Perform dump or check only for first matching application |
| `-i` | `--indent` | `<SIZE>` | Set indentation size (default: 2) |
| `-t` | `--truncate` | `<SIZE>` | Set maximum single field size (default: 30) |
| `-n` | `--include-hidden` | - | Include hidden accessibles in the tree |

# OUTPUT FORMAT

## Tree Node Format

Each node in the accessibility tree is displayed in the following format:

```
[[node_id],[role_name],[toolkit=TOOLKIT],[attributes],[x,y,width,height],[node_name],[states],[relations]]
```

## Output Tables

When dumping or checking a tree, additional tables may be displayed:

- **RELATIONS TABLE**: Shows relationships between accessible objects
- **ATTRIBUTES TABLE**: Shows detailed attributes for objects with long attribute strings
- **SUPPORTED ACTIONS TABLE**: Shows available actions for each accessible object

# STATE LEGEND

The AT-SPI state codes include:

| Code | State | Code | State |
|------|-------|------|-------|
| 0 | INVALID | 1 | ACTIVE |
| 2 | ARMED | 3 | BUSY |
| 4 | CHECKED | 5 | COLLAPSED |
| 6 | DEFUNCT | 7 | EDITABLE |
| 8 | ENABLED | 9 | EXPANDABLE |
| 10 | EXPANDED | 11 | FOCUSABLE |
| 12 | FOCUSED | 13 | HAS_TOOLTIP |
| 14 | HORIZONTAL | 15 | ICONIFIED |
| 16 | MULTI_LINE | 17 | MULTISELECTABLE |
| 18 | OPAQUE | 19 | PRESSED |
| 20 | RESIZABLE | 21 | SELECTABLE |
| 22 | SELECTED | 23 | SENSITIVE |
| 24 | SHOWING | 25 | SINGLE_LINE |
| 26 | STALE | 27 | TRANSIENT |
| 28 | VERTICAL | 29 | VISIBLE |
| 30 | MANAGES_DESCENDANTS | 31 | INDETERMINATE |
| 32 | REQUIRED | 33 | TRUNCATED |
| 34 | ANIMATED | 35 | INVALID_ENTRY |
| 36 | SUPPORTS_AUTOCOMPLETION | 37 | SELECTABLE_TEXT |
| 38 | IS_DEFAULT | 39 | VISITED |
| 40 | CHECKABLE | 41 | MODAL |
| 42 | HIGHLIGHTED | 43 | HIGHLIGHTABLE |
| 44 | HAS_POPUP | 45 | READ_ONLY |

# EXIT STATUS

| Code | Description |
|------|-------------|
| 0 | Success - Successfully communicated with the AT-SPI2 bus and printed the requested outputs |
| -1 | Error - Failed to discover the accessibility bus, failed D-Bus communication, or AT-SPI infrastructure initialization failed |

# ENVIRONMENT

- `AT_SPI_BUS_ADDRESS`: D-Bus address for the accessibility bus

# EXAMPLES

## List all accessible applications

```bash
$ at_spi2_tool -l
```

## Print the accessibility tree for an application

```bash
$ at_spi2_tool -d quickpanel
```

## Print accessibility tree with custom indentation

```bash
$ at_spi2_tool -i3 -d quickpanel
```

## Check accessibility tree integrity

```bash
$ at_spi2_tool -c starter
```

## Check tree with one-space indentation

```bash
$ at_spi2_tool -i1 -c starter
```

## Print AT-SPI state legend

```bash
$ at_spi2_tool -g
```

## Enable AT-SPI client

```bash
$ at_spi2_tool -a true
```

## Include hidden accessible objects

```bash
$ at_spi2_tool -n -d quickpanel
```

# SEE ALSO

atspi(3), dbus(1)

# PACKAGE

at-spi2-core
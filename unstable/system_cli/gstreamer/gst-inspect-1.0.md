# gst-inspect-1.0 - GStreamer registry inspection tool

## NAME

**gst-inspect-1.0** - Print information about GStreamer plugins and elements.

## SYNOPSIS

```bash
gst-inspect-1.0 [OPTIONS] [ELEMENT-NAME | PLUGIN-NAME]
```

## DESCRIPTION

**gst-inspect-1.0** is a tool to inspect the GStreamer registry. It can list all available plugins and elements, display detailed information about specific elements or plugins, and check for the existence of elements or plugins.

When run without arguments, it prints a list of all available elements. When given an element or plugin name, it displays detailed information about that element or plugin.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-a`, `--print-all` | Print all elements (detailed output). |
| `-b`, `--print-blacklist` | Print list of blacklisted files. |
| `--print-plugin-auto-install-info` | Print a machine-parsable list of features the specified plugin or all plugins provide. Useful in connection with external automatic plugin installation mechanisms. |
| `--plugin` | List the plugin contents. |
| `-t`, `--types=TYPE/KLASS` | A slashes ('/') separated list of types of elements (also known as klass) to list. (unordered) |
| `--exists` | Check if the specified element or plugin exists. |
| `--atleast-version=VERSION` | When checking if an element or plugin exists, also check that its version is at least the version specified. |
| `-u`, `--uri-handlers` | Print supported URI schemes, with the elements that implement them. |
| `--no-colors` | Disable colors in output. You can also achieve the same by setting 'GST_INSPECT_NO_COLORS' environment variable to any value. |
| `--sort=SORT-KEY` | Sort plugins and features. Sorting keys: name (default), none. |
| `-C`, `--color` | Color output, even when not sending to a tty. |
| `--version` | Print version information. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Element or plugin exists (when using `--exists`). |
| `1` | Element or plugin does not exist (when using `--exists`). |
| `-1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### List all available elements

```bash
$ gst-inspect-1.0
```

### Print all elements with details

```bash
$ gst-inspect-1.0 -a
```

### Inspect a specific element

```bash
$ gst-inspect-1.0 playbin
```

### Inspect a specific plugin

```bash
$ gst-inspect-1.0 --plugin coreelements
```

### Check if an element exists

```bash
$ gst-inspect-1.0 --exists playbin
```

### Check if element exists with minimum version

```bash
$ gst-inspect-1.0 --exists --atleast-version=1.18 playbin
```

### List elements of specific class

```bash
$ gst-inspect-1.0 -t Audio/Encoder
```

### Print URI handlers

```bash
$ gst-inspect-1.0 -u
```

### Print blacklisted plugins

```bash
$ gst-inspect-1.0 -b
```

### Print machine-parsable feature list

```bash
$ gst-inspect-1.0 --print-plugin-auto-install-info
```

### Inspect a plugin file directly

```bash
$ gst-inspect-1.0 /usr/lib/gstreamer-1.0/libgstcoreelements.so
```

## OUTPUT SECTIONS

When inspecting an element, the output includes:

### Factory Details
- Rank (primary, secondary, marginal, or none)
- Long name
- Class/Klass
- Description
- Author
- Documentation URI

### Plugin Details
- Name
- Description
- Filename
- Version
- License
- Source module
- Release date
- Binary package
- Origin URL

### Element Properties
- Property name
- Flags (readable, writable, deprecated, controllable)
- Type
- Default value
- Range (for numeric types)

### Pad Templates
- Direction (SRC, SINK)
- Availability (Always, Sometimes, On request)
- Capabilities

### Signals and Actions
- Signal name
- Return type
- Parameters

## ENVIRONMENT VARIABLES

| Variable | Description |
|----------|-------------|
| `GST_INSPECT_NO_COLORS` | Disable colored output when set to any value. |
| `GST_LESS` | Options to pass to the less pager (default: "RXF"). |
| `PAGER` | Pager program to use (default: less). |

## NOTES

- Output is automatically piped through a pager (less) on Unix systems when stdout is a TTY.
- The tool supports colored output which helps distinguish different sections and data types.
- When checking existence with `--atleast-version`, the version format is MAJOR.MINOR.MICRO.

## SEE ALSO

**gst-launch-1.0**(1), **gst-typefind-1.0**(1), **gst-stats-1.0**(1)

## PACKAGE

`gstreamer`
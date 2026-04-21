# gst-launch-1.0 - GStreamer pipeline launcher

## NAME

**gst-launch-1.0** - Build and run a GStreamer pipeline from the command line.

## SYNOPSIS

```bash
gst-launch-1.0 [OPTIONS] PIPELINE-DESCRIPTION
```

## DESCRIPTION

**gst-launch-1.0** is a tool to build and run GStreamer pipelines from the command line. It parses a pipeline description string, constructs the pipeline, and runs it. The pipeline description follows a specific syntax that allows you to chain elements together.

The pipeline description uses a syntax where elements are separated by exclamation marks (`!`). Each element can have properties set using the format `element-name property=value`.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-t`, `--tags` | Output tags (also known as metadata). |
| `-c`, `--toc` | Output TOC (table of contents - chapters and editions). |
| `-v`, `--verbose` | Output status information and property notifications. |
| `-q`, `--quiet` | Do not print any progress information. |
| `-m`, `--messages` | Output messages from the pipeline bus. |
| `-X`, `--exclude=PROPERTY-NAME` | Do not output status information for the specified property if verbose output is enabled (can be used multiple times). |
| `-f`, `--no-fault` | Do not install a fault handler. |
| `-e`, `--eos-on-shutdown` | Force EOS (End of Stream) on sources before shutting the pipeline down. |
| `--no-position` | Do not print current position of pipeline. If this option is unspecified, the position will be printed when stdout is a TTY. |
| `--force-position` | Allow printing current position of pipeline even if stdout is not a TTY. This option has no effect if the `--no-position` option is specified. |
| `--version` | Print version information. |

## PIPELINE SYNTAX

The pipeline description uses the following syntax:

- **Element chain**: Elements are chained using `!`
  ```
  element1 ! element2 ! element3
  ```

- **Properties**: Set properties using `name=value`
  ```
  element-name property=value
  ```

- **Named pads**: Link specific pads using `.` notation
  ```
  element1.src_pad ! element2.sink_pad
  ```

- **Bins**: Use parentheses to group elements
  ```
  ( element1 ! element2 )
  ```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Pipeline executed without errors. |
| `-1` | State change failure. Pipeline failed to change state. |
| `1` | General error. Pipeline construction or execution failed. |
| `2` | Interrupt. Pipeline was interrupted by user (SIGINT). |

## EXAMPLES

### Play a video file

```bash
$ gst-launch-1.0 filesrc location=video.mp4 ! decodebin ! autovideosink
```

### Play an audio file with automatic sink

```bash
$ gst-launch-1.0 filesrc location=audio.mp3 ! decodebin ! autoaudiosink
```

### Stream from a URI

```bash
$ gst-launch-1.0 playbin uri=file:///path/to/media.mp4
```

### Display verbose output with tags

```bash
$ gst-launch-1.0 -vt filesrc location=video.mp4 ! decodebin ! autovideosink
```

### Test audio pipeline

```bash
$ gst-launch-1.0 audiotestsrc ! autoaudiosink
```

### Test video pipeline with pattern

```bash
$ gst-launch-1.0 videotestsrc pattern=snow ! autovideosink
```

### Capture and encode video

```bash
$ gst-launch-1.0 v4l2src ! video/x-raw,width=640,height=480 ! x264enc ! mp4mux ! filesink location=output.mp4
```

### Force EOS on shutdown

```bash
$ gst-launch-1.0 -e filesrc location=input.mp4 ! qtdemux ! h264parse ! mp4mux ! filesink location=output.mp4
```

### Live streaming pipeline

```bash
$ gst-launch-1.0 rtmpsrc location=rtmp://server/live/stream ! flvdemux ! h264dec ! autovideosink
```

## SIGNALS

- **SIGINT** (Ctrl+C): Interrupts the pipeline and stops execution.
- **SIGHUP**: Dumps DOT file snapshot if `GST_DEBUG_DUMP_DOT_DIR` environment variable is set.
- **SIGQUIT**: Triggers debugging spin (Unix only).

## ENVIRONMENT VARIABLES

| Variable | Description |
|----------|-------------|
| `GST_DEBUG_DUMP_DOT_DIR` | Directory to dump pipeline DOT files for debugging. |
| `GST_DEBUG` | Set GStreamer debug level and categories. |
| `GST_GL_XINITTHREADS` | Initialize X threads for GL (automatically set by gst-launch). |

## NOTES

- The tool automatically wraps top-level elements in a pipeline if not already in one.
- Position information is printed by default when stdout is a TTY.
- Buffering messages are handled automatically for non-live pipelines.
- On Windows, high-resolution timer is enabled for better timing accuracy.

## SEE ALSO

**gst-inspect-1.0**(1), **gst-typefind-1.0**(1), **gst-stats-1.0**(1), **ges-launch-1.0**(1)

## PACKAGE

`gstreamer`
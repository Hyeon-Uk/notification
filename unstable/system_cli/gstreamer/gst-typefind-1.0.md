# gst-typefind-1.0 - GStreamer media type finder

## NAME

**gst-typefind-1.0** - Use GStreamer to find the type of a file.

## SYNOPSIS

```bash
gst-typefind-1.0 [OPTIONS] FILES...
```

## DESCRIPTION

**gst-typefind-1.0** is a tool that uses GStreamer to find the media type (MIME type) of one or more files. It creates a pipeline with a filesrc element, typefind element, and fakesink to analyze the file contents and determine its media type.

The tool outputs the GStreamer caps (capabilities) string that describes the detected media type, which includes the MIME type and other relevant information.

If a directory is specified, the tool recursively processes all files within that directory.

## OPTIONS

| Option | Description |
|--------|-------------|
| `--version` | Print version information. |
| `--help` | Show help options. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. All files were processed. |
| `1` | Error. No files specified or initialization failed. |

## EXAMPLES

### Find type of a single file

```bash
$ gst-typefind-1.0 video.mp4
video.mp4 - video/quicktime
```

### Find type of multiple files

```bash
$ gst-typefind-1.0 audio.mp3 image.jpg document.pdf
audio.mp3 - audio/mpeg, mpegversion=(int)1, layer=(int)3
image.jpg - image/jpeg
document.pdf - application/pdf
```

### Find type of all files in a directory

```bash
$ gst-typefind-1.0 /path/to/media/
/path/to/media/video.mp4 - video/quicktime
/path/to/media/audio.ogg - audio/x-vorbis+ogg
/path/to/media/image.png - image/png
```

### Find type of raw audio file

```bash
$ gst-typefind-1.0 recording.raw
recording.raw - audio/x-raw, format=(string)S16LE, rate=(int)44100, channels=(int)2, layout=(string)interleaved
```

### Find type of video file

```bash
$ gst-typefind-1.0 movie.mkv
movie.mkv - video/x-matroska
```

## OUTPUT FORMAT

The output format is:
```
FILENAME - CAPS_STRING
```

Where:
- **FILENAME**: The path to the analyzed file
- **CAPS_STRING**: The GStreamer caps string describing the media type

The caps string typically includes:
- **MIME type**: The primary media type (e.g., `video/quicktime`, `audio/mpeg`)
- **Additional properties**: Format-specific details (e.g., mpegversion, channels, rate)

## ERROR HANDLING

When typefinding fails, the tool outputs:
```
FILENAME - FAILED: error message
```

Common failure reasons:
- File does not exist
- File is not readable
- Unknown or unsupported media format
- File is corrupted or incomplete

## NOTES

- The tool uses GStreamer's typefind element which analyzes file contents rather than relying on file extensions.
- For directories, the tool recursively processes all files.
- The typefind element may need to read a portion of the file to determine its type.
- If no type is found, the output will indicate "No type found".

## SEE ALSO

**gst-launch-1.0**(1), **gst-inspect-1.0**(1)

## PACKAGE

`gstreamer`
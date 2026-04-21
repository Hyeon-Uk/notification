# ges-launch-1.0 - GStreamer Editing Services timeline launcher

## NAME

**ges-launch-1.0** - Create, play, or render a multimedia timeline using GStreamer Editing Services.

## SYNOPSIS

```bash
ges-launch-1.0 [OPTIONS] [TIMELINE-DESCRIPTION]
```

## DESCRIPTION

**ges-launch-1.0** creates a multimedia timeline and plays it back, or renders it to the specified format. It can load a timeline from an existing project, or create one using the timeline description format.

By default, ges-launch-1.0 is in "playback-mode" and will play the timeline. With the `--outputuri` option, it will render the timeline to a file.

## OPTIONS

### Main Options

| Option | Description |
|--------|-------------|
| `--disable-mixing` | Do not use mixing elements to mix layers together. |
| `-t`, `--track-types=<track-types>` | Specify the track types to be created (audio, video, or both). |
| `--video-caps=<caps>` | Specify the track restriction caps of the video track. |
| `--audio-caps=<caps>` | Specify the track restriction caps of the audio track. |
| `--no-interactive` | Disable interactive control via the keyboard. |

### Project Options

| Option | Description |
|--------|-------------|
| `-l`, `--load=<path>` | Load project from file. The project can be saved again with the `--save` option. |
| `-s`, `--save=<path>` | Save project to file before rendering. It can then be loaded with the `--load` option. |
| `--save-only=<path>` | Same as save project, except exit as soon as the timeline is saved instead of playing it back. |
| `--embed-nesteds` | Embed nested timelines when saving. |

### Rendering Options

| Option | Description |
|--------|-------------|
| `-o`, `--outputuri=<URI>` | Render the timeline instead of playing it back. The outputuri extension will be used to determine an encoding format. |
| `-f`, `--format=<profile>` | Set an encoding profile on the command line. |
| `-e`, `--encoding-profile=<profile-name>` | Set an encoding profile from a preset file. |
| `--profile-from=<clip-name>` | Use clip with name to determine the topology and profile of the rendered output. |
| `--forward-tags` | Forward tags from input files to the output. |
| `--smart-rendering` | Avoid reencoding when rendering. This option implies `--disable-mixing`. |

### Playback Options

| Option | Description |
|--------|-------------|
| `-v`, `--videosink=<videosink>` | Set the videosink used for playback. |
| `-a`, `--audiosink=<audiosink>` | Set the audiosink used for playback. |
| `-m`, `--mute` | Mute playback output. This has no effect when rendering. |

### Informative Options

| Option | Description |
|--------|-------------|
| `--list-transitions` | List all valid transition types and exit. |
| `--inspect-action-type` | Inspect the available action types that can be defined in a scenario. |

### Validate Options

| Option | Description |
|--------|-------------|
| `--set-test-file=<path>` | Set a test file path for gst-validate. |
| `--set-scenario=<name>` | Set a scenario name for gst-validate. |
| `--enable-validate` | Run inside GstValidate. |

## INTERACTIVE KEYBOARD CONTROLS

When running in interactive mode (default for playback), the following keyboard shortcuts are available:

| Key | Action |
|-----|--------|
| `space` | Pause/unpause |
| `q` or `ESC` | Quit |
| `→` | Seek forward |
| `←` | Seek backward |
| `+` | Increase playback rate |
| `-` | Decrease playback rate |
| `t` | Enable/disable trick modes |
| `s` | Change subtitle track |
| `0` | Seek to beginning |
| `k` | Show keyboard shortcuts |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### Play a simple timeline

```bash
$ ges-launch-1.0 +clip video.mp4
```

### Create a timeline with multiple clips

```bash
$ ges-launch-1.0 +clip video1.mp4 +clip video2.mp4
```

### Render a timeline to file

```bash
$ ges-launch-1.0 -o output.mp4 +clip video.mp4
```

### Load a project and play it

```bash
$ ges-launch-1.0 -l project.xges
```

### Load and save a project

```bash
$ ges-launch-1.0 -l project.xges -s project_new.xges
```

### Render with specific encoding format

```bash
$ ges-launch-1.0 -o output.mp4 -f "video/x-h264,profile=high" +clip video.mp4
```

### Smart rendering (avoid re-encoding)

```bash
$ ges-launch-1.0 -o output.mp4 --smart-rendering +clip video.mp4
```

### Play with custom video sink

```bash
$ ges-launch-1.0 -v "glimagesink" +clip video.mp4
```

### List available transitions

```bash
$ ges-launch-1.0 --list-transitions
```

## TIMELINE DESCRIPTION FORMAT

The timeline description format allows creating timelines on the command line:

- `+clip <uri>` - Add a clip to the timeline
- `+test-clip <pattern>` - Add a test clip (e.g., "snow", "smpte")
- `+transition <type>` - Add a transition between clips
- `+title <text>` - Add a title clip
- `layer <priority>` - Switch to a different layer

## NOTES

- The tool requires GStreamer Editing Services (GES) to be installed.
- For smart rendering to work, input files must have compatible encoding formats.
- Interactive mode is disabled when rendering or when `--no-interactive` is specified.

## SEE ALSO

**gst-launch-1.0**(1), **gst-inspect-1.0**(1)

## PACKAGE

`gst-editing-services`
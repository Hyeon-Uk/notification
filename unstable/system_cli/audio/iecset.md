# iecset - IEC958 (S/PDIF) control utility

## NAME

**iecset** - Set or dump IEC958 (S/PDIF) status bits.

## SYNOPSIS

```bash
iecset [options] [cmd arg...]
```

## DESCRIPTION

**iecset** is an ALSA utility to set or dump the IEC958 (S/PDIF) status bits. It is mostly used for digital audio interfaces (like optical/coaxial audio) to toggle professional vs consumer modes, copyright bits, and emphasis.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-D` | Specifies the control device to use. |
| `-c` | Specifies the card number to use (equiv. to `-Dhw:#`). |
| `-n` | Specifies the control index number. Default: `0` |
| `-x` | Dump the AESx hex code for IEC958 PCM parameters. |
| `-i` | Read commands from stdin. |

## STATUS BIT ARGUMENTS

### General Settings

| Argument | Values | Description |
|----------|--------|-------------|
| `professional` | `off`, `on` | Consumer or professional mode. |
| `audio` | `on`, `off` | Audio or non-audio mode. |
| `rate` | Hz value | Sample rate (e.g., 44100, 48000, 0 = not indicated). |
| `emphasis` | `0`, `1`, `2` | 0=none, 1=50/15us, 2=CCITT. |

### Professional Mode Settings

| Argument | Values | Description |
|----------|--------|-------------|
| `lock` | `off`, `on` | Rate unlocked or locked. |
| `sbits` | `2`, `4` | Sample bits: 2=20bit, 4=24bit. |

### Consumer Mode Settings

| Argument | Values | Description |
|----------|--------|-------------|
| `category` | `0`-`0x7f` | Category code. |
| `copyright` | `off`, `on` | Non-copyright or copyright. |
| `original` | `off`, `on` | 1st-gen copy or original. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command sent successfully and bits mapped. |
| `1` | Error. Device does not support IEC958 or permission denied. |

## EXAMPLES

### View IEC958 settings

```bash
$ iecset -c 0
```

### Set to consumer mode

```bash
$ iecset -c 0 professional off
```

### Enable non-audio mode (AC3/DTS passthrough)

```bash
$ iecset -c 0 audio off
```

### Set sample rate

```bash
$ iecset -c 0 rate 48000
```

### Dump AES hex code

```bash
$ iecset -c 0 -x
```

### Set copyright bit

```bash
$ iecset -c 0 copyright on
```

## NOTES

- Reaches directly to the sound card's driver controls via the specific `iec958` ALSA interface.
- Outputs the currently parsed IEC958 status bits in human-readable format.
- Use `-x` for AES hex string output.

## SEE ALSO

**amixer**(1), **aplay**(1), **alsactl**(1)

## PACKAGE

`alsa-utils`
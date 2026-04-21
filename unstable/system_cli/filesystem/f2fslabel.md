# f2fslabel - Get or set F2FS filesystem label

## NAME

**f2fslabel** - Get or set the label of an F2FS filesystem.

## SYNOPSIS

```bash
f2fslabel [OPTIONS] <device> [label]
```

## DESCRIPTION

**f2fslabel** displays or changes the filesystem label on an F2FS filesystem. If no label is provided, the current label is displayed.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-V`, `--version` | Show version. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Get current label

```bash
$ f2fslabel /dev/sda1
mydata
```

### Set new label

```bash
$ f2fslabel /dev/sda1 newlabel
```

### Clear label

```bash
$ f2fslabel /dev/sda1 ""
```

## NOTES

- Maximum label length is 512 characters.
- The filesystem should be unmounted when changing the label.
- Labels are useful for identifying filesystems.

## SEE ALSO

**mkfs.f2fs**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
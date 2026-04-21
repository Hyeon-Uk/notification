# fibmap.f2fs - Get file block mapping for F2FS

## NAME

**fibmap.f2fs** - Get file block mapping information on F2FS filesystem.

## SYNOPSIS

```bash
fibmap.f2fs <file>
```

## DESCRIPTION

**fibmap.f2fs** displays the logical-to-physical block mapping for a file on an F2FS filesystem. It shows which physical blocks contain the file data.

## OPTIONS

This command has no options.

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Get block mapping

```bash
$ fibmap.f2fs /mnt/f2fs/file.txt
file: /mnt/f2fs/file.txt
logical offset        physical block
0                     12345
1                     12346
2                     12347
```

## NOTES

- File must be on an F2FS filesystem.
- Shows physical block numbers for each logical block.
- Useful for debugging file allocation.

## SEE ALSO

**f2fs_io**(8), **dump.f2fs**(8)

## PACKAGE

`f2fs-tools`
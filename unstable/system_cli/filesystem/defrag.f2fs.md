# defrag.f2fs - Defragment F2FS files

## NAME

**defrag.f2fs** - Defragment files on an F2FS filesystem.

## SYNOPSIS

```bash
defrag.f2fs [OPTIONS] <path>
```

## DESCRIPTION

**defrag.f2fs** defragments a file or directory on an F2FS filesystem. It rearranges file data to be contiguous, which can improve read performance.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-s <offset>`, `--start <offset>` | Start offset (in blocks). |
| `-l <length>`, `--length <length>` | Length to defragment (in blocks). |
| `-t <blocks>`, `--target <blocks>` | Target block address. |
| `-i`, `--interactive` | Interactive mode. |
| `-f`, `--force` | Force defragmentation. |
| `-v`, `--verbose` | Verbose output. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Defragment a file

```bash
$ defrag.f2fs /mnt/f2fs/file.txt
```

### Defragment with specific range

```bash
$ defrag.f2fs -s 0 -l 1000 /mnt/f2fs/file.txt
```

### Defragment directory

```bash
$ defrag.f2fs /mnt/f2fs/directory/
```

## NOTES

- Works on mounted F2FS filesystem.
- Defragmentation can improve sequential read performance.
- May not be effective for SSDs due to wear leveling.
- Requires F2FS filesystem support.

## SEE ALSO

**f2fs_io**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
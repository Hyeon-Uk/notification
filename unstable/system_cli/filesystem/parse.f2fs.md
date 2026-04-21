# parse.f2fs - Parse F2FS filesystem structures

## NAME

**parse.f2fs** - Parse and display F2FS filesystem structures.

## SYNOPSIS

```bash
parse.f2fs [OPTIONS] <device>
```

## DESCRIPTION

**parse.f2fs** is a debugging tool that parses and displays the internal structures of an F2FS filesystem. It can show various metadata structures for analysis.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-s`, `--super` | Parse superblock. |
| `-c`, `--checkpoint` | Parse checkpoint. |
| `-n`, `--node` | Parse node segments. |
| `-d`, `--data` | Parse data segments. |
| `-S`, `--sit` | Parse SIT (Segment Information Table). |
| `-a`, `--ssa` | Parse SSA (Segment Summary Area). |
| `-v`, `--verbose` | Verbose output. |
| `-h`, `--help` | Show help. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |

## EXAMPLES

### Parse superblock

```bash
$ parse.f2fs -s /dev/sda1
```

### Parse all structures

```bash
$ parse.f2fs -s -c -S /dev/sda1
```

## NOTES

- This is a debugging tool.
- Filesystem can be mounted or unmounted.
- Useful for forensic analysis.

## SEE ALSO

**dump.f2fs**(8), **fsck.f2fs**(8)

## PACKAGE

`f2fs-tools`
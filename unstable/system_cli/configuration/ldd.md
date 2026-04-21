# ldd - Print shared library dependencies

## NAME

**ldd** - Print shared library dependencies for a program.

## SYNOPSIS

```bash
ldd [options] file...
```

## DESCRIPTION

**ldd** prints the shared libraries required by each program or shared library specified on the command line. It is useful for determining which libraries a binary depends on and whether all required libraries are available.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Print all information, including symbol versioning information. |
| `-u`, `--unused` | Print unused direct dependencies. |
| `-d`, `--data-relocs` | Process relocations and report any missing objects. |
| `-r`, `--function-relocs` | Process data and function relocations and report missing objects or functions. |
| `--version` | Print ldd version. |
| `-h`, `--help` | Print help message. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. All libraries were found. |
| `1` | Error. Missing libraries or invalid arguments. |
| `2` | Error. Invalid option. |
| `3` | Error. Object not found or not executable. |

## EXAMPLES

### Check library dependencies

```bash
$ ldd /usr/bin/ls
```

### Verbose output

```bash
$ ldd -v /usr/bin/ls
```

### Find unused dependencies

```bash
$ ldd -u /usr/bin/ls
```

### Check for missing relocations

```bash
$ ldd -d /usr/bin/ls
```

### Check a shared library

```bash
$ ldd /usr/lib/libc.so.6
```

## NOTES

- Output shows library name, address (if resolved), and whether found.
- `linux-vdso.so` is a virtual library provided by the kernel.
- `not found` indicates a missing dependency.
- May not work correctly with setuid/setgid programs.
- Alternative: `objdump -p file | grep NEEDED` for safer inspection.

## SEE ALSO

**ldconfig**(1), **ld.so**(8), **objdump**(1), **readelf**(1)

## PACKAGE

`glibc`
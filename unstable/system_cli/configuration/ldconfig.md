# ldconfig - Configure dynamic linker run-time bindings

## NAME

**ldconfig** - Configure the dynamic linker run-time bindings.

## SYNOPSIS

```bash
ldconfig [options]
```

## DESCRIPTION

**ldconfig** creates the necessary links and cache to the most recent shared libraries found in the directories specified on the command line, in the file `/etc/ld.so.conf`, and in the trusted directories `/lib` and `/usr/lib`. The cache is used by the run-time linker to find shared libraries quickly.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Verbose mode. Print current version number, name of each directory, and links created. |
| `-n` | Process only directories specified on command line. Don't process trusted directories or `/etc/ld.so.conf`. |
| `-N` | Don't rebuild the cache. |
| `-X` | Don't update symbolic links. |
| `-f conf` | Use `conf` instead of `/etc/ld.so.conf`. |
| `-C cache` | Use `cache` instead of `/etc/ld.so.cache`. |
| `-r root` | Change to and use `root` as the root directory. |
| `-l` | Library mode. Manually link individual libraries. |
| `-p`, `--print-cache` | Print the lists of directories and candidate libraries stored in the current cache. |
| `-c format` | Use the specified format for the cache file. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or permission denied. |

## EXAMPLES

### Update library cache

```bash
$ ldconfig
```

### Verbose cache update

```bash
$ ldconfig -v
```

### Print current cache

```bash
$ ldconfig -p
```

### Process specific directory

```bash
$ ldconfig -n /usr/local/lib
```

### Use alternative root

```bash
$ ldconfig -r /mnt/root
```

## NOTES

- Must be run as root to update the cache.
- Reads library paths from `/etc/ld.so.conf` and `/etc/ld.so.conf.d/`.
- Creates `/etc/ld.so.cache` for fast library lookups.
- Run after installing new shared libraries.

## SEE ALSO

**ldd**(1), **ld.so**(8)

## PACKAGE

`glibc`
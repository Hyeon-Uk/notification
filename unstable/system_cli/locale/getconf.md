# getconf

**NAME**

getconf - Get system configuration variables

**SYNOPSIS**

```bash
getconf [OPTIONS] VARIABLE
getconf [OPTIONS] -a
```

**DESCRIPTION**

`getconf` displays the value of system configuration variables defined by POSIX, system-specific variables, or variables from the pathname based on the specified path.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-a` | Display all configuration variables and their values |
| `-v SPEC` | Use specification SPEC for variables |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**COMMON VARIABLES**

| Variable | Description |
|----------|-------------|
| PATH_MAX | Maximum path length |
| NAME_MAX | Maximum filename length |
| ARG_MAX | Maximum argument length |
| OPEN_MAX | Maximum open files |
| CHILD_MAX | Maximum simultaneous processes |
| CLK_TCK | Clock ticks per second |
| PAGESIZE | System page size |
| LONG_BIT | Number of bits in a long |
| WORD_BIT | Number of bits in a word |

**EXAMPLES**

1. Display all configuration variables:

```bash
getconf -a
```

2. Get maximum path length:

```bash
getconf PATH_MAX
```

3. Get maximum filename length:

```bash
getconf NAME_MAX /tmp
```

4. Get page size:

```bash
getconf PAGESIZE
```

5. Get maximum argument length:

```bash
getconf ARG_MAX
```

**PATHNAME VARIABLES**

Some variables can be queried for specific paths:
- LINK_MAX - Maximum hard links
- MAX_CANON - Maximum canonical input line length
- MAX_INPUT - Maximum input queue length
- NAME_MAX - Maximum filename length
- PATH_MAX - Maximum path length
- PIPE_BUF - Pipe buffer size

**SEE ALSO**

sysconf(3), pathconf(3), fpathconf(3)

**PACKAGE**

glibc
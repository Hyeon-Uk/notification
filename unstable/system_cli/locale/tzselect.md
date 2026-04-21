# tzselect

**NAME**

tzselect - Select a time zone

**SYNOPSIS**

```bash
tzselect
```

**DESCRIPTION**

`tzselect` is an interactive utility that helps users select an appropriate time zone. It prompts the user with a series of questions about their location and outputs the resulting time zone name.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-c COORD` | Use coordinates (LAT LON) to find time zone |
| `-n LIMIT` | Limit search radius (with -c) |
| `-h`, `--help` | Display help message |

**EXAMPLES**

1. Run interactive time zone selection:

```bash
tzselect
```

2. Select time zone by coordinates:

```bash
tzselect -c "+37.5 +127.0"
```

**INTERACTIVE PROCESS**

The program will prompt:
1. Select a continent or ocean
2. Select a country or region
3. Select a time zone from the list

**OUTPUT**

After selection, tzselect prints the time zone name (e.g., `Asia/Seoul`) which can be used:
- As the value for the TZ environment variable
- To create the /etc/localtime file

**FILES**

- `/usr/share/zoneinfo/` - Time zone data files
- `/usr/share/zoneinfo/zone1970.tab` - Time zone table

**SEE ALSO**

zic(1), zdump(1), tzfile(5)

**PACKAGE**

tzdata
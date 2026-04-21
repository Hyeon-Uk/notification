# zdump

**NAME**

zdump - Time zone dumper

**SYNOPSIS**

```bash
zdump [OPTIONS] [TIMEZONE...]
```

**DESCRIPTION**

`zdump` prints the current time in each time zone named on the command line, or detailed information about the time zone transitions.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-v`, `--verbose` | Print detailed information about time transitions |
| `-V`, `--version` | Display version information |
| `-c [END_YEAR]` | Cutoff at END_YEAR (default: current year + 10) |
| `-i`, `--interrupted` | Use interrupted output format |
| `-s`, `--silent` | Output only warnings |
| `-h`, `--help` | Display help message |

**EXAMPLES**

1. Display current time in UTC:

```bash
zdump UTC
```

2. Display current time in multiple time zones:

```bash
zdump America/New_York Europe/London Asia/Seoul
```

3. Display verbose transition information:

```bash
zdump -v America/New_York
```

4. Display transitions with cutoff year:

```bash
zdump -v -c 2030 America/Los_Angeles
```

**OUTPUT FORMAT**

Standard output format:
```
TIMEZONE  DAY  MON  DD  HH:MM:SS  YYYY
```

Verbose output includes:
- Time before and after transitions
- Whether daylight saving time is in effect
- The time zone abbreviation

**FILES**

- `/usr/share/zoneinfo/` - Timezone data files

**SEE ALSO**

tzselect(1), zic(1), tzfile(5)

**PACKAGE**

tzdata
# zic

**NAME**

zic - Time zone compiler

**SYNOPSIS**

```bash
zic [OPTIONS] [FILE...]
```

**DESCRIPTION**

`zic` reads text files describing time zones and compiles them into the binary format used by the system for time zone information. These binary files are typically stored in `/usr/share/zoneinfo/`.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-d DIR`, `--directory=DIR` | Output directory for compiled files |
| `-l LOCALTIME`, `--localtime=LOCALTIME` | Set default local time zone |
| `-p POSIXRULES`, `--posixrules=POSIXRULES` | Set POSIX rules time zone |
| `-L FILE`, `--leapseconds=FILE` | Include leap seconds from FILE |
| `-v`, `--verbose` | Verbose output |
| `-s`, `--silent` | Silent mode |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Compile timezone files:

```bash
zic africa asia europe
```

2. Compile to specific directory:

```bash
zic -d /usr/share/zoneinfo timezone_file
```

3. Set default timezone:

```bash
zic -l America/New_York northamerica
```

**INPUT FILE FORMAT**

Each line in the input file can be one of:
- Rule - Define daylight saving time rules
- Zone - Define time zone
- Link - Create alias for a time zone
- Leap - Define leap seconds

**FILES**

- `/usr/share/zoneinfo/` - Compiled timezone files
- `/etc/localtime` - Local timezone file (often a symlink)
- `/usr/share/zoneinfo/zone.tab` - Timezone table

**SEE ALSO**

tzselect(1), zdump(1), tzfile(5)

**PACKAGE**

tzdata
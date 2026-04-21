# gencat

**NAME**

gencat - Generate a message catalog from a message text file

**SYNOPSIS**

```bash
gencat [OPTIONS] OUTPUT_FILE [INPUT_FILE...]
```

**DESCRIPTION**

`gencat` generates a message catalog from one or more input text files. The message catalog is used by programs for internationalization and localization using the catgets(3) interface.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-H`, `--header=NAME` | Create C header file with definitions |
| `--new` | Remove old messages not in input file |
| `-o FILE`, `--output=FILE` | Output file name |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**INPUT FILE FORMAT**

Each message in the input file follows this format:

```
$set SET_ID
MSG_ID MESSAGE_TEXT
```

Where:
- `SET_ID` - Set identifier (integer)
- `MSG_ID` - Message identifier (integer)
- `MESSAGE_TEXT` - The actual message text

**EXAMPLES**

1. Create a message catalog:

```bash
gencat messages.cat messages.txt
```

2. Create catalog with header file:

```bash
gencat -H messages.h messages.cat messages.txt
```

3. Merge multiple input files:

```bash
gencat messages.cat set1.txt set2.txt set3.txt
```

**FILES**

- `/usr/share/locale/*/LC_MESSAGES/` - Message catalog location

**SEE ALSO**

catgets(3), catopen(3), catclose(3)

**PACKAGE**

glibc
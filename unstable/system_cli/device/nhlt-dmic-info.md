# nhlt-dmic-info - NHLT DMIC information parser

## NAME

**nhlt-dmic-info** - Parse and display NHLT (Non-HDA Link Table) DMIC (Digital Microphone) information.

## SYNOPSIS

```bash
nhlt-dmic-info [options]
```

## DESCRIPTION

**nhlt-dmic-info** is a utility to parse and display NHLT (Non-HDA Link Table) DMIC information from ACPI tables. The NHLT table contains hardware link configuration for audio devices, particularly digital microphones on Intel platforms.

The tool reads the NHLT ACPI table and outputs DMIC configuration information in JSON format.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message. |
| `-v`, `--version` | Print version of the program. |
| `-f`, `--file #` | Specify NHLT file path. Default: `/sys/firmware/acpi/tables/NHLT`. |
| `-o`, `--output #` | Specify output file path. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments or operation failed. |

## EXAMPLES

### Display NHLT DMIC information

```bash
$ nhlt-dmic-info
```

### Use custom NHLT file

```bash
$ nhlt-dmic-info -f /path/to/NHLT
```

### Save output to file

```bash
$ nhlt-dmic-info -o dmic_config.json
```

## NOTES

- NHLT (Non-HDA Link Table) is an ACPI table for Intel platforms.
- The tool is primarily used for audio hardware configuration debugging.
- Requires appropriate permissions to read ACPI tables.

## SEE ALSO

**amixer**(1), **aplay**(1)

## PACKAGE

`alsa-utils`
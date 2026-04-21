# json-glib-validate

**NAME**

json-glib-validate - JSON file validation tool

**SYNOPSIS**

```bash
json-glib-validate [OPTIONS] FILE...
```

**DESCRIPTION**

`json-glib-validate` is a utility for validating the syntax of JSON files. It reads JSON data from the specified URI or file path and verifies that the data follows proper JSON syntax.

This tool is part of the json-glib library, providing JSON data parsing and validation capabilities. When invalid JSON data is found, it outputs an error message and returns a non-zero exit code.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Display help message. |

**EXAMPLES**

1. Validate a single JSON file:

```bash
json-glib-validate data.json
```

2. Validate multiple JSON files:

```bash
json-glib-validate file1.json file2.json file3.json
```

3. Check validation result (no output on success):

```bash
json-glib-validate valid.json && echo "Valid JSON"
```

4. Validate an invalid JSON file:

```bash
json-glib-validate invalid.json
# Error message is displayed and exit code 1 is returned
```

**EXIT STATUS**

| Code | Description |
|------|-------------|
| 0 | All files are valid JSON format |
| 1 | One or more files are invalid or file read error occurred |

**DIAGNOSTICS**

- For valid JSON files, no output is produced and exit code 0 is returned.
- For invalid JSON files, an error message is printed to stderr and exit code 1 is returned.
- If a file does not exist or cannot be read, an error message is displayed.

**SEE ALSO**

json-glib-format(1)

**PACKAGE**

json-glib
# json-glib-format

**NAME**

json-glib-format - JSON file formatting tool

**SYNOPSIS**

```bash
json-glib-format [OPTIONS] FILE...
```

**DESCRIPTION**

`json-glib-format` is a utility for formatting JSON files. It reads JSON data and adds indentation and whitespace to improve readability, or saves the output to a specified file.

This tool is part of the json-glib library, providing JSON data parsing and generation capabilities.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Display help message. |
| `-p`, `--prettify` | Prettify the output. Adds indentation and whitespace to improve JSON data readability. |
| `-i SPACES`, `--indent-spaces=SPACES` | Specify the number of spaces for indentation. Default is 2. This option only takes effect when used with `--prettify`. |
| `-o FILE`, `--output=FILE` | Save output to the specified file. If not specified, output goes to stdout. |

**EXAMPLES**

1. Format a JSON file with prettified output:

```bash
json-glib-format --prettify data.json
```

2. Format with 4-space indentation:

```bash
json-glib-format --prettify --indent-spaces=4 data.json
```

3. Save formatted output to another file:

```bash
json-glib-format --prettify --output=formatted.json data.json
```

4. Format multiple JSON files:

```bash
json-glib-format --prettify file1.json file2.json file3.json
```

5. Convert JSON file to compact form (remove indentation):

```bash
json-glib-format data.json > compact.json
```

**EXIT STATUS**

| Code | Description |
|------|-------------|
| 0 | Successfully completed |
| 1 | Error occurred (file not found, parsing error, etc.) |

**SEE ALSO**

json-glib-validate(1)

**PACKAGE**

json-glib
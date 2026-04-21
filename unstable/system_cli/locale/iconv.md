# iconv

**NAME**

iconv - Convert text from one character encoding to another

**SYNOPSIS**

```bash
iconv [OPTIONS] [-f FROM_ENCODING] [-t TO_ENCODING] [FILE...]
```

**DESCRIPTION**

`iconv` converts text from one character encoding to another. It reads input from files (or standard input) and writes the converted output to standard output.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-f ENCODING`, `--from-code=ENCODING` | Source encoding |
| `-t ENCODING`, `--to-code=ENCODING` | Target encoding |
| `-l`, `--list` | List all supported encodings |
| `-c` | Silently discard characters that cannot be converted |
| `-s`, `--silent` | Suppress warnings |
| `-v`, `--verbose` | Print progress information |
| `-o FILE`, `--output=FILE` | Output file |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**EXAMPLES**

1. Convert from UTF-8 to ISO-8859-1:

```bash
iconv -f UTF-8 -t ISO-8859-1 input.txt > output.txt
```

2. Convert to UTF-8 (auto-detect input encoding):

```bash
iconv -t UTF-8 input.txt > output.txt
```

3. List all supported encodings:

```bash
iconv -l
```

4. Convert with transliteration:

```bash
iconv -f UTF-8 -t ASCII//TRANSLIT input.txt > output.txt
```

5. Convert and save to output file:

```bash
iconv -f UTF-8 -t EUC-KR -o output.txt input.txt
```

**COMMON ENCODINGS**

| Encoding | Description |
|----------|-------------|
| UTF-8 | Unicode UTF-8 |
| UTF-16 | Unicode UTF-16 |
| ISO-8859-1 | Latin-1 (Western European) |
| EUC-KR | Korean (Extended Unix Code) |
| EUC-JP | Japanese (Extended Unix Code) |
| GB2312 | Chinese simplified |
| ASCII | 7-bit ASCII |

**EXIT STATUS**

| Code | Description |
|------|-------------|
| 0 | Successful conversion |
| 1 | Invalid encoding specified |
| 2 | Unable to open input file |
| 3 | Conversion error |

**SEE ALSO**

iconvconfig(1), iconv(3)

**PACKAGE**

glibc
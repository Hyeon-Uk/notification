# xsltproc Specification

## 1. Overview
- **CLI Path:** `/usr/bin/xsltproc`
- **Execution Model:** One-shot
- **Description:** Standard upstream XML transformation utility applying XSLT stylesheets to structurally format XML documents heavily.

## 2. Command Syntax
```bash
/usr/bin/xsltproc [OPTIONS] [STYLESHEET] [XML_FILE]
```

## 3. Global Options
Standard upstream flags (`-o` output, `--stringparam`).

## 4. Commands (if applicable)
Procedurally parses document matrices.

## 5. Environment Variables & Configuration
- Native LibXML paths.

## 6. Output Format
- **Format:** Text / XML
- **Structure Details:** Outputs the resultant XML or HTML payload directly based on the stylesheet transform schema.

## 7. Exit Status Codes
| Code | Name | Description |
|------|------|-------------|
| `0` | Success | The operation completed successfully. |
| `<nonzero>` | General Error | Execution failed. |

## 8. Practical Examples
### Example 1: Translate XML
**Context:** Process raw XML config into HTML safely.
```bash
$ xsltproc style.xsl data.xml > output.html
```

# dlog_redirect_stdout Specification

## 1. Overview
- **CLI Path:** `/usr/bin/dlog_redirect_stdout`
- **Execution Model:** **Wrapper** (wraps another executable)
- **Description:** `dlog_redirect_stdout` launches a child process and redirects its stdout and/or stderr into DLog ring buffers. Used to capture console output from applications that write to stdout/stderr and route them into the structured DLog system.

## 2. Command Syntax
```bash
dlog_redirect_stdout [options] -- /full/path/to/executable [args]
```
The `--` separator is required between redirector options and the child process.

## 3. Global Options

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `--outtag TAG` | String | `STDOUT_<path>` | Tag for stdout logs. Enables stdout redirection. |
| `--outpriority P` | Char | `I` | DLog priority for stdout: `V`, `D`, `I`, `W`, `E`, `F`. |
| `--outbuffer BUFFER` | String | `main` | DLog buffer for stdout: `main`, `system`, `radio`, `apps`. |
| `--errtag TAG` | String | `STDERR_<path>` | Tag for stderr logs. Enables stderr redirection. |
| `--errpriority P` | Char | `E` | DLog priority for stderr. |
| `--errbuffer BUFFER` | String | `main` | DLog buffer for stderr. |
| `--nofallback` | Boolean | `false` | Don't run the app if redirection setup fails. |

## 4. Commands (if applicable)
Single-purpose wrapper command.

## 5. Environment Variables & Configuration
- Uses DLog socket infrastructure.

## 6. Output Format
- **Format:** Transparent pass-through
- **Structure Details:** The child process executes normally; stdout/stderr go to DLog buffers instead.

## 7. Exit Status Codes

| Code | Name | Description |
|------|------|-------------|
| `0` | Success | Child process exited with 0. |
| `>0` | Error | Child exit code, or setup failure with `--nofallback`. |

## 8. Practical Examples

### Example 1: Redirect script output to DLog
```bash
$ dlog_redirect_stdout --outtag MY_SCRIPT --outpriority I -- /usr/bin/my_script.sh
```

### Example 2: Redirect both stdout and stderr
```bash
$ dlog_redirect_stdout --outtag MYAPP --errtag MYAPP_ERR --errbuffer system -- /usr/bin/myapp
```

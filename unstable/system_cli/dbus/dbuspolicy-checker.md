# dbuspolicy-checker - D-Bus policy file checker

## NAME

**dbuspolicy-checker** - A light-weight checker for D-Bus policy configuration files.

## SYNOPSIS

```bash
dbuspolicy-checker [-v] [-p] [-s|-u|filename]
```

## DESCRIPTION

**dbuspolicy-checker** is a light-weight checker for D-Bus policy configuration files. It checks both syntax and semantics, looking for common errors in D-Bus policy configuration files.

The tool validates D-Bus policy files against the specification and reports errors, warnings, and potential issues that could cause security problems or runtime failures.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-s` | Check system bus policy configuration. |
| `-u` | Check session bus policy configuration. |
| `-d` | Enable verbose mode. |
| `-v` | Include every ISO XSLs. |
| `-p` | Enable profile mode. |
| `filename` | Path to a D-Bus policy configuration file to check. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. No errors found in policy file. |
| `1` | Error. Invalid arguments or errors found in policy file. |

## EXAMPLES

### Check a specific policy file

```bash
$ dbuspolicy-checker /etc/dbus-1/system.d/myapp.conf
```

### Check system bus policy

```bash
$ dbuspolicy-checker -s
```

### Check session bus policy

```bash
$ dbuspolicy-checker -u
```

### Check with verbose output

```bash
$ dbuspolicy-checker -d /etc/dbus-1/system.d/myapp.conf
```

### Check with profile mode

```bash
$ dbuspolicy-checker -p /etc/dbus-1/system.d/myapp.conf
```

## NOTES

- D-Bus policy files are XML files that define security policies for D-Bus connections.
- Policy files are typically located in `/etc/dbus-1/system.d/` for system bus or `/etc/dbus-1/session.d/` for session bus.
- Common errors checked include invalid syntax, unknown attributes, and security policy violations.
- The checker helps identify potential security issues before deploying applications.

## SEE ALSO

**dbus-daemon**(1), **dbus-send**(1)

## PACKAGE

`dbus-tools`
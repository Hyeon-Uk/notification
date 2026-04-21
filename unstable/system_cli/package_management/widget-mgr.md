# widget-mgr

## NAME

widget-mgr - Widget management command-line tool

## SYNOPSIS

```
widget-mgr [options] <command> [arguments]
```

## DESCRIPTION

`widget-mgr` is a command-line tool for managing web widgets on Tizen systems. It provides operations for installing, uninstalling, listing, and querying widget packages (WGT files).

## OPTIONS

| Option | Description |
|--------|-------------|
| `-i` | Install a widget |
| `-u` | Uninstall a widget |
| `-l` | List installed widgets |
| `-s` | Show widget information |
| `-h` | Print help message |

## USAGE EXAMPLES

### Install a Widget

```bash
widget-mgr -i /tmp/org.example.widget.wgt
```

### Uninstall a Widget

```bash
widget-mgr -u org.example.widget
```

### List Installed Widgets

```bash
widget-mgr -l
```

### Show Widget Information

```bash
widget-mgr -s org.example.widget
```

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| Non-zero | Error occurred |

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/apps/` | Widget installation directory |
| `/opt/usr/dbspace/.widget.db` | Widget database |

## SEE ALSO

- [pkgcmd](pkgcmd.md) - Package manager command-line tool
- [wgt-backend](wgt-backend.md) - Widget backend service
- [widget_recovery](widget_recovery.md) - Widget recovery tool

## PACKAGE

`widget-manager` (Tizen package name)
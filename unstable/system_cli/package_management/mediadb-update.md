# mediadb-update

## NAME

mediadb-update - Update Tizen media database by scanning directories

## SYNOPSIS

```
mediadb-update <directory_path | command>
```

## DESCRIPTION

`mediadb-update` is a Tizen-specific tool for updating the media database. It scans specified directories for media files and updates the media database to reflect new, modified, or removed media content.

This tool is typically used after installing or removing packages that contain media content, or when media files have been manually added or removed.

## USAGE

| Argument | Description |
|----------|-------------|
| `<directory_path>` | Directory path to scan for media files (must be under `/opt/usr/content` or `/opt/usr/storage`) |
| `check_db` | Check media database and create tables if they do not exist |
| `clean` | Remove all values from the folder and media tables |

## EXAMPLES

### Scan a directory for media files

```bash
mediadb-update /opt/usr/content
```

### Scan user content directory

```bash
mediadb-update /opt/usr/content/Music
```

### Check and repair media database

```bash
mediadb-update check_db
```

### Clean media database

```bash
mediadb-update clean
```

## DIRECTORY REQUIREMENTS

The directory path must be under one of the following locations:

| Location | Description |
|----------|-------------|
| `/opt/usr/content` | User content directory (TZ_USER_CONTENT) |
| `/opt/usr/storage` | System storage directory (TZ_SYS_STORAGE) |

## PROCESS

1. **Directory validation**: Verifies the specified path is a valid directory
2. **Storage ID lookup**: Determines the storage ID for the specified path
3. **Directory scanning**: Scans the directory recursively for media files
4. **Database update**: Updates the media database with discovered content

## FILES

| File | Description |
|------|-------------|
| `/opt/usr/dbspace/.media.db` | Media database file |
| `/usr/lib/libmedia-content-plugin.so` | Media content plugin library |

## EXIT STATUS

| Status | Description |
|--------|-------------|
| 0 | Success |
| 0 | Invalid path (help displayed) |

## NOTES

- The tool requires exactly one argument
- Invalid paths will display the help message
- The tool runs asynchronously for directory scanning
- Progress is reported with elapsed time upon completion

## SEE ALSO

- [pkgcmd](pkgcmd.md) - Package management command
- [pkginfo](pkginfo.md) - Package information tool

## PACKAGE

`media-server` (Tizen package name)
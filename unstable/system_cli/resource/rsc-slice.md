# RSC-SLICE

**NAME**

rsc-slice - resource slicing tool for Tizen applications

**SYNOPSIS**

```
rsc-slice -p PATH [OPTIONS]
rsc-slice -p PATH -d DPI_VALUE
```

**DESCRIPTION**

`rsc-slice` is a command-line utility that optimizes application resources by removing unused resource files based on device characteristics. It analyzes the resource directory structure and removes files that are not needed for the specified device configuration, helping to reduce application package size.

The tool uses the AUL (Application Utility Library) resource manager to determine which resource files are valid for the given device configuration and deletes the rest.

**OPTIONS**

| Option | Long Form | Argument | Description |
|--------|-----------|----------|-------------|
| `-p` | `--path` | PATH | Set the resource directory path to process |
| `-d` | `--screen-dpi` | DPI | Set the screen DPI value for resource filtering |

**ARGUMENTS**

- **PATH**: Absolute or relative path to the resource directory containing application resources to be sliced.
- **DPI**: Screen DPI (dots per inch) value used to filter resources. Common values include 120 (ldpi), 160 (mdpi), 240 (hdpi), 320 (xhdpi), 480 (xxhdpi), and 640 (xxxhdpi).

**OPERATION**

1. The tool initializes the resource manager with the specified path and device configuration.
2. It builds a list of valid resource files that match the device criteria.
3. Files not in the valid list are deleted.
4. Empty directories are also removed after file deletion.

**EXAMPLES**

```
# Slice resources in a specific directory
rsc-slice -p /home/user/workspace/testapp/res/

# Slice resources with DPI filtering (300 DPI screen)
rsc-slice -p /home/user/workspace/testapp/res/ -d 300

# Slice resources using absolute path
rsc-slice -p /opt/usr/apps/org.example.app/res/ -d 320
```

**RESOURCE SELECTION**

The tool supports resource selection based on various device qualifiers stored in directory names (e.g., `drawable-xhdpi`, `values-hdpi`). When DPI is specified, only resources matching that density or a compatible fallback are retained.

**EXIT STATUS**

| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Failure (invalid path, DPI value, or processing error) |

**OUTPUT**

The tool provides informational output during execution:
- The absolute path being processed
- The DPI value used for filtering
- Files being deleted (with "deleting [path]" messages)
- Total processing time in milliseconds

**SEE ALSO**

res-copy(1), aul_resource_manager(3)

**PACKAGE**

platform/core/appfw/pkgmgr-tool
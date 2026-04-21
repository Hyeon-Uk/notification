# dotnettool

## NAME

dotnettool - .NET native image and TAC management tool

## SYNOPSIS

```
dotnettool [options] [command] [arguments]
```

## DESCRIPTION

dotnettool is a command-line tool for managing native images (NI), Trusted Assembly Cache (TAC), and profiles on the Tizen .NET platform. It improves the execution performance of .NET applications through AOT (Ahead-Of-Time) compilation.

## COMMANDS

### Native Image Commands

| Command | Description |
|---------|-------------|
| `--ni-system` | Generate native images for system DLLs |
| `--ni-dll [path...]` | Generate native images for specified DLLs |
| `--ni-pkg [pkgId...]` | Generate native images in package bin/lib directories |
| `--ni-pkg-with-path [path...]` | Generate package native images from specified paths |
| `--ni-dir [dir...]` | Generate native images in specified directories |
| `--ni-reset-system` | Remove system native image files |
| `--ni-reset-all-app` | Remove all app native image files |
| `--ni-reset-pkg [pkgId...]` | Remove native image files for specified packages |
| `--ni-reset-dir [dir...]` | Remove native images in specified directories |
| `--ni-regen-all-app` | Regenerate native images for all apps |

### TAC (TLC) Commands

| Command | Description |
|---------|-------------|
| `--tac-reset-all` | Remove all TAC native image files |
| `--tac-regen-all` | Regenerate all TAC files |
| `--tac-restore-db` | Restore TAC database |
| `--tac-disable-pkg [pkgId...]` | Disable TAC for packages |
| `--tac-enable-pkg [pkgId...]` | Enable TAC for packages |

### Profile and Other Commands

| Command | Description |
|---------|-------------|
| `--resolve-all-app` | Remove unused multi-targeting files for all apps |
| `--rm-app-profile [pkgId...]` | Remove app profiles for specified packages (requires root privilege) |
| `--rm-all-app-profile` | Remove app profiles for all packages (requires root privilege) |
| `--check-all-app-privilege` | Check app privileges for all packages (requires root privilege) |
| `-h, --help` | Display help |

## OPTIONS

| Option | Description |
|--------|-------------|
| `--mibc [path]` | Specify Mibc file (separated by `:`) |
| `--verbose` | Display detailed information |
| `--inputbubble` | Compile input assemblies as one bubble |
| `--inputbubbleref [path]` | Add reference files to bubble |
| `--ref [path]` | Compile reference files (system path auto-configured) |
| `--no-pipeline` | Compile DLLs individually |
| `--print-cmd` | Print command and options |
| `--skip-ro-app` | Skip NI regeneration for apps installed in RO area |
| `--rm-origin-after-ni` | Remove original DLL after native image generation |
| `--set-priority [value]` | Set process priority (-20 to 19, default: 0) |
| `--parallelism [num]` | Maximum number of threads for AOTC (default: number of logical processors) |
| `--timeout [sec]` | Child process termination timeout (default: none) |
| `--force` | Enable TAC/TLC for apps without TAC metadata |
| `--instruction-set [set]` | Instruction set for crossgen2 compilation (default: native) |

## EXAMPLES

Generate system native images:
```
$ dotnettool --ni-system
```

Generate native images for a DLL:
```
$ dotnettool --ni-dll /usr/bin/Tizen.Runtime.dll
```

Generate native images for a package:
```
$ dotnettool --ni-pkg org.tizen.FormsGallery
```

Regenerate native images for all .NET packages:
```
$ dotnettool --ni-regen-all-app
```

Generate native images based on Mibc data:
```
$ dotnettool --mibc /tmp/ibcdata/in.mibc --ni-dll /usr/bin/Tizen.Runtime.dll
```

Remove package profile:
```
$ dotnettool --rm-app-profile org.tizen.FormsGallery
```

Enable TAC for a package:
```
$ dotnettool --tac-enable-pkg org.tizen.FormsGallery
```

Run in verbose mode:
```
$ dotnettool --verbose --ni-pkg org.tizen.FormsGallery
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| -1 | Error occurred |

## NOTES

- The `--rm-origin-after-ni` option cannot be used with App AOTC options
- TAC-related commands are for app performance optimization
- Do not use the `--inputbubble` option without proper understanding

## SEE ALSO

- dotnet(1)
- crossgen2(1)

## PACKAGE

dotnet-launcher
# TIZEN-KEY-MANAGER-CLI(1)

## NAME
**tizen-key-manager-cli** - Inspect and manage application-owned aliases in
the Tizen Key Manager secure store.

## SYNOPSIS
**tizen-key-manager-cli** *COMMAND* [*ARGS*]  
**tizen-key-manager-cli** **-h** | **--help** | **help**  
**tizen-key-manager-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-key-manager-cli** provides command-line access to the native Tizen
Key Manager client APIs from `ckmc/ckmc-manager.h` and `ckmc/ckmc-type.h`.

The tool focuses on application-visible alias management:
- listing stored data, keys, and certificates
- showing alias metadata such as password protection and backend
- retrieving password-protected data blobs
- storing and removing application-owned data aliases
- querying backend capabilities exposed by Key Manager

Commands that read or mutate stored content can fail when the user key is
locked, when the alias is unknown, or when the caller lacks permission to the
selected object.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **list-data** | None | Lists data aliases visible to the current application. |
| **list-data-info** | None | Lists data aliases with password-protection and backend metadata. |
| **list-keys** | None | Lists key aliases visible to the current application. |
| **list-keys-info** | None | Lists key aliases with password-protection and backend metadata. |
| **list-certs** | None | Lists certificate aliases visible to the current application. |
| **list-certs-info** | None | Lists certificate aliases with password-protection and backend metadata. |
| **backend-info** | `<sw|tz>` | Prints backend capability information for the software or TrustZone backend. |
| **get** | `<alias> [password]` | Retrieves a stored data alias and prints the decoded payload. |
| **save-password** | `<alias> <encrypt_password> <secret>` | Stores a UTF-8 secret as password-protected data under the alias. |
| **remove** | `<alias>` | Removes the alias with `ckmc_remove_alias()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, Key Manager API failure, permission failure, or
locked/unavailable secure storage state.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-key-manager-cli`

## AUTHOR
Part of the **Tizen Tools** project.

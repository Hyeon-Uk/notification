# f2fscrypt - F2FS filesystem encryption utility

## NAME

**f2fscrypt** - Manage encryption on F2FS filesystems.

## SYNOPSIS

```bash
f2fscrypt <command> [OPTIONS]
```

## DESCRIPTION

**f2fscrypt** is a utility to manage filesystem-level encryption on F2FS filesystems. It allows setting up encryption policies and keys for directories.

## COMMANDS

| Command | Description |
|---------|-------------|
| `add_key <keyspec>` | Add encryption key to filesystem. |
| `get_policy <path>` | Get encryption policy for a directory. |
| `new_session` | Add key to current keyring. |
| `set_policy <policy> <path>` | Set encryption policy on a directory. |
| `get_policy <path>` | Get encryption policy from a directory. |
| `get_status <path>` | Get encryption status of a file/directory. |

## OPTIONS

| Option | Description |
|--------|-------------|
| `-p <parent>` | Inherit policy from parent directory. |
| `-d <descriptor>` | Use specific key descriptor. |
| `-c <contents>` | Contents encryption mode. |
| `-f <filenames>` | Filenames encryption mode. |
| `-n <flags>` | Policy flags. |
| `-S <source>` | Key source (1: custom, 2: device). |
| `-s` | Use salt with key. |
| `-v`, `--verbose` | Verbose output. |
| `-h`, `--help` | Show help. |

## ENCRYPTION MODES

### Contents Encryption Modes

| Mode | Description |
|------|-------------|
| `AES-256-XTS` | AES-256 in XTS mode (default). |
| `AES-128-CBC` | AES-128 in CBC mode. |
| `AES-128-CTS` | AES-128 in CTS mode. |

### Filenames Encryption Modes

| Mode | Description |
|------|-------------|
| `AES-256-CTS` | AES-256 in CTS mode (default). |
| `AES-128-CBC` | AES-128 in CBC mode. |
| `AES-128-CTS` | AES-128 in CTS mode. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. |
| `1` | Error. |
| `2` | Invalid arguments. |

## EXAMPLES

### Add encryption key

```bash
$ f2fscrypt add_key
Enter passphrase (echo disabled): 
Added encryption key with descriptor 1234567890abcdef
```

### Set encryption policy on directory

```bash
$ f2fscrypt set_policy 1234567890abcdef /mnt/f2fs/encrypted/
Key with descriptor '1234567890abcdef' applied to /mnt/f2fs/encrypted/.
```

### Get encryption policy

```bash
$ f2fscrypt get_policy /mnt/f2fs/encrypted/
/mnt/f2fs/encrypted/: AES-256-XTS,AES-256-CTS
```

### Get encryption status

```bash
$ f2fscrypt get_status /mnt/f2fs/encrypted/
/mnt/f2fs/encrypted/: encrypted
```

## NOTES

- Requires F2FS filesystem formatted with encryption support (-O encrypt).
- Requires kernel support for fscrypt (4.2+).
- Keys are managed per-filesystem.
- Encrypted files cannot be accessed without the key.
- Lost keys cannot be recovered.

## SEE ALSO

**mkfs.f2fs**(8), **fsck.f2fs**(8), **f2fs_io**(8)

## PACKAGE

`f2fs-tools`
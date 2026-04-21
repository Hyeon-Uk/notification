# buxton2ctl - Command-line tool for Buxton2 configuration system

## NAME

**buxton2ctl** - Command-line interface for the Buxton2 security-enabled configuration management system.

## SYNOPSIS

```bash
buxton2ctl [option] [command] [arguments...]
```

## DESCRIPTION

**buxton2ctl** is the command-line interface for Buxton2, a security-enabled configuration management system. It provides a layered approach to configuration storage, with each layer containing key-value pairs. Mandatory Access Control (MAC) is implemented at the key-value level.

Buxton2 uses a daemon (buxtond) for processing client requests and enforcing MAC. The CLI can operate either through the daemon or directly access the database.

## OPTIONS

| Option | Description |
|--------|-------------|
| `-c`, `--config-file=[FILE]` | Path to configuration file (direct access only). |
| `-d`, `--direct` | Directly access the database without daemon. |
| `-i`, `--install` | Execute on BASE db used for the default value. |
| `-u`, `--uid=[UID]` | Specify the UID for the operation. |
| `-m`, `--support-multi-user` | Force operation on multi-user mode. |
| `-h`, `--help` | Display help and exit. |

## COMMANDS

### Value Operations

| Command | Description |
|---------|-------------|
| `check` | Check the availability of Buxton daemon. |
| `get <layer> <key>` | Get a value by key. |
| `unset <layer> <key>` | Unset (delete) a value by key. |
| `list-keys <layer>` | List the keys for a layer. |

### Create Operations

| Command | Description |
|---------|-------------|
| `create-string <layer> <key> <value> [priv]` | Create a key with a string value. |
| `create-int32 <layer> <key> <value> [priv]` | Create a key with an int32_t value. |
| `create-uint32 <layer> <key> <value> [priv]` | Create a key with an uint32_t value. |
| `create-int64 <layer> <key> <value> [priv]` | Create a key with an int64_t value. |
| `create-uint64 <layer> <key> <value> [priv]` | Create a key with an uint64_t value. |
| `create-double <layer> <key> <value> [priv]` | Create a key with a double precision value. |
| `create-bool <layer> <key> <value> [priv]` | Create a key with a boolean value. |

### Set Operations

| Command | Description |
|---------|-------------|
| `set-string <layer> <key> <value>` | Set a key with a string value. |
| `set-int32 <layer> <key> <value>` | Set a key with an int32_t value. |
| `set-uint32 <layer> <key> <value>` | Set a key with an uint32_t value. |
| `set-int64 <layer> <key> <value>` | Set a key with an int64_t value. |
| `set-uint64 <layer> <key> <value>` | Set a key with an uint64_t value. |
| `set-double <layer> <key> <value>` | Set a key with a double precision value. |
| `set-bool <layer> <key> <value>` | Set a key with a boolean value. |

### Privilege Operations

| Command | Description |
|---------|-------------|
| `get-read-priv <layer> <key>` | Get a value's read privilege. |
| `set-read-priv <layer> <key> <priv>` | Set a value's read privilege. |
| `get-write-priv <layer> <key>` | Get a value's write privilege. |
| `set-write-priv <layer> <key> <priv>` | Set a value's write privilege. |

### Debug Operations

| Command | Description |
|---------|-------------|
| `enable-debug` | Enable buxton debug mode. |
| `disable-debug` | Disable buxton debug mode. |
| `security-enable` | Enable security check. |
| `security-disable` | Disable security check. |

### User Operations

| Command | Description |
|---------|-------------|
| `user-list` | List available users. |
| `user-switch <user>` | Switch to a user. |
| `user-current` | Get current user. |

## LAYERS

Buxton2 uses a layered approach to configuration storage. Common layers include:

- `system` - System-level configuration
- `user` - User-specific configuration
- `memory` - In-memory configuration (non-persistent)
- `user_memory` - User-specific in-memory configuration

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. Invalid arguments, key not found, or operation failed. |

## EXAMPLES

### Check Buxton availability

```bash
$ buxton2ctl check
```

### Get a value

```bash
$ buxton2ctl get system bluetooth/status
bluetooth/status = 1 (int32)
```

### Set a string value

```bash
$ buxton2ctl set-string system home/language "en_US"
```

### Set an empty string

```bash
$ buxton2ctl set-string system home/language ""
```

### Set a negative integer value

```bash
$ buxton2ctl set-int32 system wifi/status -1
```

### Set a boolean value

```bash
$ buxton2ctl set-bool system bluetooth/enabled true
```

### Create a new key with privilege

```bash
$ buxton2ctl create-string system myapp/config "value" "User::App::myapp"
```

### List keys in a layer

```bash
$ buxton2ctl list-keys system
```

### Unset a key

```bash
$ buxton2ctl unset system myapp/config
```

### Direct access mode

```bash
$ buxton2ctl -d get system bluetooth/status
```

### List available users

```bash
$ buxton2ctl user-list
```

### Switch user

```bash
$ buxton2ctl user-switch guest
```

### Get current user

```bash
$ buxton2ctl user-current
```

## NOTES

- When buxtond daemon is running, use normal mode (without `-d`).
- When daemon is not available, use `-d` for direct database access.
- Security checks are enforced by Cynara for MAC (Mandatory Access Control).
- The `-i` option is used for setting default values during installation.

## SEE ALSO

**vconftool**(1), **buxtond**(8)

## PACKAGE

`buxton2`
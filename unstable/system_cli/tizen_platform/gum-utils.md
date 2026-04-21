# gum-utils

## NAME

gum-utils - Tizen user and group management utility

## SYNOPSIS

```
gum-utils [OPTION...]
```

## DESCRIPTION

gum-utils is a command-line tool for managing users and groups on the Tizen platform. It provides functionality for adding, deleting, modifying, and querying users and groups. It supports both online mode via D-Bus and offline mode for direct operation without gumd.

**Note:** Only one command can be executed at a time.

## OPTIONS

### Action Options (Application Options)

| Option | Description |
|--------|-------------|
| `-a, --add-user` | Add a user (username/nickname and usertype required) |
| `-d, --delete-user` | Delete a user (uid required) |
| `-u, --update-user` | Update user information (uid required; modifiable: secret, realname, office, officephone, homephone, icon, shell) |
| `-b, --get-user` | Query a user (uid required) |
| `-c, --get-user-by-name` | Query a user by name (username required) |
| `-g, --add-group` | Add a group (groupname and grouptype required) |
| `-h, --delete-group` | Delete a group (gid required) |
| `-i, --update-group` | Update group information (gid required; modifiable: secret) |
| `-j, --get-group` | Query a group (gid required) |
| `-k, --get-group-by-name` | Query a group by name (groupname required) |
| `-m, --add-member` | Add a member to a group (gid and mem_uid required) |
| `-n, --delete-member` | Delete a member from a group (gid and mem_uid required) |
| `-p, --write-nfc` | Write username and secret to NFC tag (used during user creation/modification) |
| `-o, --offline` | Offline mode (call libgum sync API directly without gumd) |
| `-q, --sysroot=sysroot` | System root path (offline mode only) |
| `-r, --user-list` | Query user list (returns only specified types if usertypes is set) |

### User Service Options

| Option | Description |
|--------|-------------|
| `--usertypes=type` | User type filter (system, admin, guest, normal). Multiple types can be specified with comma (e.g., normal,system) |

### User Options

| Option | Description |
|--------|-------------|
| `--username=name` | User name |
| `--usertype=type` | User type (system, admin, guest, normal) |
| `--uid=uid` | User ID |
| `--ugid=gid` | Group ID |
| `--usecret=secret` | Password (plaintext) |
| `--nickname=nickname` | Nickname |
| `--realname=name` | Real name |
| `--office=office` | Office location |
| `--officephone=phone` | Office phone number |
| `--homephone=phone` | Home phone number |
| `--homedir=dir` | Home directory |
| `--shell=shell` | Shell path |
| `--icon=icon` | Icon path |

### Group Options

| Option | Description |
|--------|-------------|
| `--groupname=name` | Group name |
| `--grouptype=type` | Group type (system, user) |
| `--gid=gid` | Group ID |
| `--gsecret=secret` | Group password (plaintext) |
| `--mem_uid=uid` | Member user ID |

### Help Options

| Option | Description |
|--------|-------------|
| `-?, --help` | Show help |
| `--help-all` | Show all help |
| `--help-user-service-options` | Show user service options help |
| `--help-user-options` | Show user options help |
| `--help-group-options` | Show group options help |

## EXAMPLES

Add a normal user (online mode):
```
$ gum-utils -a --username=user1 --usertype=normal
```

Delete a user in offline mode:
```
$ gum-utils -o -d --uid=2001
```

Query a user:
```
$ gum-utils -b --uid=5001
```

Query a user by name:
```
$ gum-utils -c --username=testuser
```

Update user information:
```
$ gum-utils -u --uid=5001 --realname="Test User" --office="R&D Center"
```

Query all users:
```
$ gum-utils -r
```

Query only specific user types:
```
$ gum-utils -r --usertypes=normal,system
```

Add a group:
```
$ gum-utils -g --groupname=developers --grouptype=user
```

Add a member to a group:
```
$ gum-utils -m --gid=1001 --mem_uid=5001
```

Use a specific sysroot in offline mode:
```
$ gum-utils -o -r --sysroot=/mnt/root
```

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| 0 | Success |
| 1 | Error occurred |

## SEE ALSO

- gumd(8)
- useradd(8)
- groupadd(8)

## PACKAGE

gum-utils
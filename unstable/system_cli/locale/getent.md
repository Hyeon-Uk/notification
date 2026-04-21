# getent

**NAME**

getent - Get entries from administrative databases

**SYNOPSIS**

```bash
getent [OPTIONS] DATABASE [KEY...]
```

**DESCRIPTION**

`getent` retrieves entries from system administrative databases specified by the database name. It uses the system's name service switch configuration to determine the source of the data.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-s SERVICE` | Use specified service (e.g., files, dns, nis) |
| `-h`, `--help` | Display help message |
| `-V`, `--version` | Display version information |

**DATABASES**

| Database | Description |
|----------|-------------|
| passwd | User accounts |
| group | Group accounts |
| hosts | Host names and addresses |
| services | Network services |
| protocols | Network protocols |
| networks | Network names and addresses |
| ethers | Ethernet addresses |
| rpc | RPC program names and numbers |
| netgroup | Network groups |
| shadow | Shadow passwords |
| aliases | Mail aliases |

**EXAMPLES**

1. Get all user entries:

```bash
getent passwd
```

2. Get specific user entry:

```bash
getent passwd root
```

3. Get all group entries:

```bash
getent group
```

4. Get host entry by name:

```bash
getent hosts localhost
```

5. Get host entry by IP:

```bash
getent hosts 127.0.0.1
```

6. Get service entry:

```bash
getent services ssh
```

**SEE ALSO**

nsswitch.conf(5), passwd(5), group(5), hosts(5)

**PACKAGE**

glibc
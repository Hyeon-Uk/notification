# NSJAIL.SH

**NAME**

nsjail.sh - wrapper script for nsjail sandbox execution

**SYNOPSIS**

```
nsjail.sh COMMAND [ARGS...]
```

**DESCRIPTION**

`nsjail.sh` is a wrapper script that provides pre-configured nsjail sandbox execution for Tizen applications. It sets up appropriate namespace isolation and resource limits before executing the specified command.

**EXAMPLES**

```
# Run application in sandbox
nsjail.sh /usr/bin/myapp

# Run with arguments
nsjail.sh /bin/cat /etc/hosts
```

**SEE ALSO**

nsjail(1)

**PACKAGE**

Tizen-specific wrapper script
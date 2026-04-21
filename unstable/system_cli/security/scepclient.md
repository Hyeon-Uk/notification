# SCEPCLIENT

**NAME**

scepclient - SCEP (Simple Certificate Enrollment Protocol) client

**SYNOPSIS**

```
scepclient [OPTIONS]
```

**DESCRIPTION**

`scepclient` is a command-line utility for enrolling certificates using the Simple Certificate Enrollment Protocol (SCEP). It communicates with a SCEP server to request and obtain certificates for device authentication.

**OPTIONS**

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |
| `-s, --server URL` | SCEP server URL |
| `-c, --cert FILE` | Certificate file |
| `-k, --key FILE` | Private key file |
| `-o, --output FILE` | Output certificate file |

**EXAMPLES**

```
# Enroll certificate from SCEP server
scepclient -s http://scep.example.com/scep -o cert.pem

# Request certificate with existing key
scepclient -s http://scep.example.com/scep -k key.pem -o cert.pem
```

**PACKAGE**

Tizen platform tool
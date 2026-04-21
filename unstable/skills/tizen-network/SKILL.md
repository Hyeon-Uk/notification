---
name: tizen-network
description: Manages network connectivity, HTTP requests, Wi-Fi operations, and network service discovery. Use when working with network connections, HTTP clients, Wi-Fi Aware/Direct, or network monitoring.
---

# Tizen Network

Manages network connectivity, HTTP requests, Wi-Fi operations, and service discovery.

## Quick reference

**Connection**: Connection management → `tizen-connection-cli`
**HTTP**: HTTP client → `tizen-http-cli`
**NSD**: Network Service Discovery → `tizen-nsd-cli`
**URL Download**: URL downloads → `tizen-url-download-cli`
**Wi-Fi Aware**: Wi-Fi Aware → `tizen-wifi-aware-cli`
**Wi-Fi Direct**: Wi-Fi Direct → `tizen-wifi-direct-cli`

## Connection

Monitor and manage network connections:

```bash
# Check connection status
tizen-connection-cli status

# List available networks
tizen-connection-cli list

# Connect to network
tizen-connection-cli connect --ssid MyNetwork --password pass123
```

See [tizen-connection-cli.md](references/tizen-connection-cli.md) for complete documentation.

## HTTP

Make HTTP requests:

```bash
# GET request
tizen-http-cli get --url http://example.com/api/data

# POST request
tizen-http-cli post --url http://example.com/api --data '{"key":"value"}'

# Download file
tizen-http-cli download --url http://example.com/file.zip --output file.zip
```

See [tizen-http-cli.md](references/tizen-http-cli.md) for complete documentation.

## NSD (Network Service Discovery)

Discover network services:

```bash
# Create local service template
tizen-nsd-cli local-template --type _http._tcp --name demo --port 8080

# List service states
tizen-nsd-cli list-states

# Create remote template
tizen-nsd-cli remote-template --type _http._tcp --name demo
```

See [tizen-nsd-cli.md](references/tizen-nsd-cli.md) for complete documentation.

## URL Download

Download files from URLs:

```bash
# Download file
tizen-url-download-cli download --url http://example.com/file.txt --output file.txt

# Resume interrupted download
tizen-url-download-cli resume --url http://example.com/file.zip --partial partial.zip
```

See [tizen-url-download-cli.md](references/tizen-url-download-cli.md) for complete documentation.

## Wi-Fi Aware

Manage Wi-Fi Aware (NAN) connections:

```bash
# Start Wi-Fi Aware
tizen-wifi-aware-cli start

# Discover peers
tizen-wifi-aware-cli discover --service-name my-service

# Publish service
tizen-wifi-aware-cli publish --service-name my-service
```

See [tizen-wifi-aware-cli.md](references/tizen-wifi-aware-cli.md) for complete documentation.

## Wi-Fi Direct

Manage Wi-Fi Direct (P2P) connections:

```bash
# Scan for devices
tizen-wifi-direct-cli scan

# Connect to peer
tizen-wifi-direct-cli connect --mac-address AA:BB:CC:DD:EE:FF

# Disconnect
tizen-wifi-direct-cli disconnect
```

See [tizen-wifi-direct-cli.md](references/tizen-wifi-direct-cli.md) for complete documentation.

## Common use cases

- **Network monitoring**: Monitor connectivity and network status
- **HTTP clients**: Make REST API calls and download content
- **Service discovery**: Find and advertise network services
- **P2P communication**: Use Wi-Fi Aware/Direct for peer-to-peer connections

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Use `--help` flag with any CLI tool for detailed usage information.
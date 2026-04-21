# TIZEN-TELEPHONY-CLI(1)

## NAME
**tizen-telephony-cli** - Query primary SIM, network, and modem status
through Tizen telephony APIs.

## SYNOPSIS
**tizen-telephony-cli** *COMMAND*  
**tizen-telephony-cli** **-h** | **--help** | **help**  
**tizen-telephony-cli** **-V** | **--version** | **version**

## DESCRIPTION
**tizen-telephony-cli** provides command-line access to the one-shot
Telephony information APIs exposed by `telephony_sim.h`,
`telephony_network.h`, and `telephony_modem.h`.

The tool opens the primary telephony handle returned by
`telephony_init()`, performs a single query, prints structured text
output, and deinitializes the handle before exit.

Most commands require the `%http://tizen.org/privilege/telephony`
privilege. Runtime support also depends on the device image exposing a
usable telephony stack.

## COMMANDS REFERENCE

| Command | Args | Description |
| :--- | :--- | :--- |
| **sim-state** | None | Prints the current primary SIM state returned by `telephony_sim_get_state()`. |
| **sim-info** | None | Prints public SIM details including operator, MSIN, SPN, subscriber number, subscriber ID, and SIM application mask. |
| **network-info** | None | Prints public network details including name, MCC, MNC, roaming status, RSSI, network type, packet-service type, and service state. |
| **modem-status** | None | Prints the modem power status returned by `telephony_modem_get_power_status()`. |

## EXIT STATUS
**0**: Success.  
**1**: Invalid arguments, telephony initialization failure, permission
failure, unsupported runtime, or query failure.

## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/tizen-telephony-cli`

## AUTHOR
Part of the **Tizen Tools** project.

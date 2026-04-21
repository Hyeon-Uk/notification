---
name: smartthings-curl
description: Use when the user wants to inspect SmartThings locations, rooms, devices, or device state, or send supported device commands through curl and jq with a skill-local credential file.
---

# SmartThings Curl

Use this skill for SmartThings setup, device discovery, state inspection, and supported device control with `curl` and `jq`.

## Files
- `state/credentials.env`: skill-local credential file
- `scripts/setup_smartthings.sh`: PAT or OAuth credential bootstrap and auth-code exchange
- `scripts/smartthings_api.sh`: reusable SmartThings API helper

## Workflow
1. Confirm `curl` and `jq` are available.
2. If `state/credentials.env` does not contain usable credentials, run `scripts/setup_smartthings.sh`.
3. Prefer `SMARTTHINGS_PAT` or `SMARTTHINGS_TOKEN` for direct SmartThings API calls.
4. If the user already has a SmartThings OAuth app, save `SMARTTHINGS_CLIENT_ID` and `SMARTTHINGS_CLIENT_SECRET`, print the authorize URL, then rerun setup with `--auth-code`.
5. For device actions, resolve the device first, inspect its current state, then send only commands backed by exposed capabilities.

## Rules
- Keep SmartThings secrets in `state/credentials.env`.
- Do not `source` the credential file in the shell. Scope values can contain spaces; use the bundled scripts, which parse the file safely.
- The API helper prefers `SMARTTHINGS_PAT` or `SMARTTHINGS_TOKEN` and falls back to `SMARTTHINGS_ACCESS_TOKEN`.
- If an OAuth access token is near expiry and refresh credentials exist, the helper refreshes it automatically.
- Resolve a device by name before acting. Use location and room filters when the user provides them.
- Inspect capabilities or current status before assuming commands.
- Prefer `locations`, `rooms`, `list-devices`, `find-device`, `device-status`, `device-capabilities`, `capability-schema`, and `device-command` over raw HTTP.
- This curl/jq variant does not create a SmartThings OAuth app. If the user needs OAuth without a PAT, they must create the app separately and store the client credentials in `state/credentials.env`.

## Useful commands
```bash
bash scripts/setup_smartthings.sh --pat YOUR_PAT
bash scripts/setup_smartthings.sh --client-id YOUR_CLIENT_ID --client-secret YOUR_CLIENT_SECRET
bash scripts/setup_smartthings.sh --auth-code YOUR_CODE

bash scripts/smartthings_api.sh locations
bash scripts/smartthings_api.sh rooms --location-name "Home"
bash scripts/smartthings_api.sh find-device --name "TV" --location-name "Home"
bash scripts/smartthings_api.sh device-status --device-id DEVICE_ID
bash scripts/smartthings_api.sh device-capabilities --device-id DEVICE_ID
bash scripts/smartthings_api.sh capability-schema --capability switch --version 1
bash scripts/smartthings_api.sh device-command --device-id DEVICE_ID --capability switch --command on
```

## TV app discovery
- App launch is device-specific. Confirm the TV exposes the relevant launch capability before claiming support.
- When a user asks to open a TV app, inspect current TV status for fields such as `installedApps`, `supportedApps`, or the current app identifier.
- Do not assume app ids such as `org.tizen.netflix-app` are portable across devices.

## Notes
- The setup script is headless and does not open a browser.
- Create a PAT here: https://account.smartthings.com/tokens
- Device support depends on the capabilities exposed by each device in SmartThings.

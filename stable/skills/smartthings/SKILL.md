---
name: smartthings
description: Use when the user wants to set up SmartThings credentials, discover devices, inspect device state, or send SmartThings commands through this skill's bundled scripts and local state file.
---

# SmartThings

Use this skill for SmartThings setup, device discovery, state inspection, and supported device control.

## Files
- `state/credentials.env`: skill-local credential file
- `scripts/setup_smartthings.py`: one-time OAuth app setup and token exchange
- `scripts/smartthings_api.py`: reusable SmartThings API helper

## Workflow
1. If `state/credentials.env` does not contain usable credentials, run `scripts/setup_smartthings.py`.
2. For setup, keep a PAT in `SMARTTHINGS_PAT` or `SMARTTHINGS_TOKEN` inside `state/credentials.env`.
3. Run setup once, open the printed login URL on the user's phone, copy the `code` query parameter from the redirect URL, then rerun with `--auth-code`.
4. For device actions, resolve the device first, inspect its current state, then send only commands backed by exposed capabilities.

## Rules
- Keep SmartThings secrets in `state/credentials.env`.
- Do not `source` the credential file in the shell. Scope values can contain spaces; use the bundled scripts, which parse the file safely.
- The API helper prefers `SMARTTHINGS_PAT` or `SMARTTHINGS_TOKEN` for direct SmartThings API calls and falls back to `SMARTTHINGS_ACCESS_TOKEN` when needed.
- Prefer `scripts/smartthings_api.py` for all reads and writes.
- Resolve a device by name before acting. Use location and room filters when the user provides them.
- Inspect capabilities or current status before assuming commands.
- Use `list-devices`, `find-device`, `device-status`, `device-capabilities`, `capability-schema`, `device-command`, `locations`, and `rooms` rather than raw HTTP when possible.
- If PAT-based app creation fails with `403`, create the app with another SmartThings CLI login flow, write `SMARTTHINGS_CLIENT_ID` and `SMARTTHINGS_CLIENT_SECRET` into `state/credentials.env`, and rerun the auth-code step.
- Re-running setup is safe; it updates `state/credentials.env` in place.

## Useful commands
```bash
python scripts/smartthings_api.py locations
python scripts/smartthings_api.py rooms --location-name "Home"
python scripts/smartthings_api.py find-device --name "TV" --location-name "Home"
python scripts/smartthings_api.py device-status --device-id DEVICE_ID
python scripts/smartthings_api.py device-capabilities --device-id DEVICE_ID
python scripts/smartthings_api.py capability-schema --capability switch --version 1
python scripts/smartthings_api.py device-command --device-id DEVICE_ID --capability switch --command on
```

## TV app discovery
- App launch is device-specific. Confirm the TV exposes the relevant launch capability before claiming support.
- When a user asks to open a TV app, first inspect current TV status for fields such as `installedApps`, `supportedApps`, or the current app identifier.
- Do not assume app ids such as `org.tizen.netflix-app` are portable across devices.

## Notes
- The setup script is headless and does not open a browser.
- Create a PAT here: https://account.smartthings.com/tokens
- Device support depends on the capabilities exposed by each device in SmartThings.

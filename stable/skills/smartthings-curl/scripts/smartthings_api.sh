#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=./common.sh
source "$SCRIPT_DIR/common.sh"

usage() {
  cat <<'EOF'
Usage:
  smartthings_api.sh [--config PATH] <command> [options]

Commands:
  locations
  rooms --location-id ID | --location-name NAME
  list-devices [--location-id ID] [--room-id ID]
  find-device --name NAME [--location-id ID | --location-name NAME] [--room-id ID | --room-name NAME] [--exact]
  device-status --device-id ID
  device-command --device-id ID --capability CAP --command CMD [--component NAME] [--arg VALUE ...]
EOF
}

TOKEN=""

print_api_error() {
  local payload="$1"
  if printf '%s' "$payload" | jq empty >/dev/null 2>&1; then
    jq -n --argjson body "$payload" --arg status "${ST_HTTP_STATUS:-}" '{
      error:{
        status: ($status | if . == "" then null else tonumber end),
        body:$body
      }
    }' | jq .
  else
    jq -n --arg body "$payload" --arg status "${ST_HTTP_STATUS:-}" '{
      error:{
        status: ($status | if . == "" then null else tonumber end),
        body:$body
      }
    }' | jq .
  fi
}

resolve_token_or_die() {
  local config="$1"
  TOKEN="$(st_resolve_api_token "$config")" || true
  if [[ -z "$TOKEN" ]]; then
    st_json_error "No SmartThings token found. Add SMARTTHINGS_PAT or run setup_smartthings.sh." | jq .
    return 1
  fi
}

find_location_id() {
  local token="$1"
  local location_name="$2"
  local payload

  payload="$(st_api_request GET /v1/locations "$token")" || {
    print_api_error "$payload"
    return 1
  }

  printf '%s' "$payload" | jq -r --arg name "$location_name" '
    .items[]
    | select(.name == $name)
    | .locationId
  ' | head -n 1
}

find_room_id() {
  local token="$1"
  local location_id="$2"
  local room_name="$3"
  local payload

  payload="$(st_api_request GET "/v1/locations/$location_id/rooms" "$token")" || {
    print_api_error "$payload"
    return 1
  }

  printf '%s' "$payload" | jq -r --arg name "$room_name" '
    .items[]
    | select(.name == $name)
    | .roomId
  ' | head -n 1
}

main() {
  local config command token payload output
  local location_id location_name room_id room_name name exact device_id capability command_name component
  local -a command_args

  st_require_cmd curl jq base64 date mktemp awk sed

  config="$(st_default_config)"
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --config)
        config="$2"
        shift 2
        ;;
      --help|-h)
        usage
        exit 0
        ;;
      locations|rooms|list-devices|find-device|device-status|device-command)
        command="$1"
        shift
        break
        ;;
      *)
        printf 'Unknown option or command: %s\n' "$1" >&2
        usage >&2
        exit 2
        ;;
    esac
  done

  if [[ -z "${command:-}" ]]; then
    usage >&2
    exit 2
  fi

  resolve_token_or_die "$config" || exit 1
  token="$TOKEN"

  case "$command" in
    locations)
      payload="$(st_api_request GET /v1/locations "$token")" || {
        print_api_error "$payload"
        exit 1
      }
      st_attach_meta "$payload"
      ;;

    rooms)
      location_id=""
      location_name=""
      while [[ $# -gt 0 ]]; do
        case "$1" in
          --location-id)
            location_id="$2"
            shift 2
            ;;
          --location-name)
            location_name="$2"
            shift 2
            ;;
          *)
            printf 'Unknown rooms option: %s\n' "$1" >&2
            exit 2
            ;;
        esac
      done
      if [[ -z "$location_id" && -n "$location_name" ]]; then
        location_id="$(find_location_id "$token" "$location_name")"
      fi
      if [[ -z "$location_id" ]]; then
        st_json_error "rooms requires --location-id or --location-name." | jq .
        exit 1
      fi
      payload="$(st_api_request GET "/v1/locations/$location_id/rooms" "$token")" || {
        print_api_error "$payload"
        exit 1
      }
      st_attach_meta "$payload"
      ;;

    list-devices)
      location_id=""
      room_id=""
      while [[ $# -gt 0 ]]; do
        case "$1" in
          --location-id)
            location_id="$2"
            shift 2
            ;;
          --room-id)
            room_id="$2"
            shift 2
            ;;
          *)
            printf 'Unknown list-devices option: %s\n' "$1" >&2
            exit 2
            ;;
        esac
      done
      payload="$(st_api_request GET /v1/devices "$token")" || {
        print_api_error "$payload"
        exit 1
      }
      output="$(printf '%s' "$payload" | jq \
        --arg location_id "$location_id" \
        --arg room_id "$room_id" '
          {
            items: [
              .items[]
              | select(($location_id == "" or .locationId == $location_id) and ($room_id == "" or .roomId == $room_id))
            ]
          }
        ')"
      st_attach_meta "$output"
      ;;

    find-device)
      name=""
      location_id=""
      location_name=""
      room_id=""
      room_name=""
      exact=0
      while [[ $# -gt 0 ]]; do
        case "$1" in
          --name)
            name="$2"
            shift 2
            ;;
          --location-id)
            location_id="$2"
            shift 2
            ;;
          --location-name)
            location_name="$2"
            shift 2
            ;;
          --room-id)
            room_id="$2"
            shift 2
            ;;
          --room-name)
            room_name="$2"
            shift 2
            ;;
          --exact)
            exact=1
            shift
            ;;
          *)
            printf 'Unknown find-device option: %s\n' "$1" >&2
            exit 2
            ;;
        esac
      done
      if [[ -z "$name" ]]; then
        st_json_error "find-device requires --name." | jq .
        exit 1
      fi
      if [[ -z "$location_id" && -n "$location_name" ]]; then
        location_id="$(find_location_id "$token" "$location_name")"
      fi
      if [[ -z "$room_id" && -n "$room_name" ]]; then
        if [[ -z "$location_id" ]]; then
          st_json_error "--room-name requires --location-id or --location-name." | jq .
          exit 1
        fi
        room_id="$(find_room_id "$token" "$location_id" "$room_name")"
      fi
      payload="$(st_api_request GET /v1/devices "$token")" || {
        print_api_error "$payload"
        exit 1
      }
      output="$(printf '%s' "$payload" | jq \
        --arg name "$name" \
        --arg location_id "$location_id" \
        --arg room_id "$room_id" \
        --argjson exact "$exact" '
          {
            items: [
              .items[]
              | select($location_id == "" or .locationId == $location_id)
              | select($room_id == "" or .roomId == $room_id)
              | select(
                  [(.label // ""), (.name // "")]
                  | any(
                      if $exact == 1 then
                        ascii_downcase == ($name | ascii_downcase)
                      else
                        ascii_downcase | contains($name | ascii_downcase)
                      end
                    )
                )
            ]
          }
        ')"
      st_attach_meta "$output"
      ;;

    device-status)
      device_id=""
      while [[ $# -gt 0 ]]; do
        case "$1" in
          --device-id)
            device_id="$2"
            shift 2
            ;;
          *)
            printf 'Unknown device-status option: %s\n' "$1" >&2
            exit 2
            ;;
        esac
      done
      if [[ -z "$device_id" ]]; then
        st_json_error "device-status requires --device-id." | jq .
        exit 1
      fi
      payload="$(st_api_request GET "/v1/devices/$device_id/status" "$token")" || {
        print_api_error "$payload"
        exit 1
      }
      st_attach_meta "$payload"
      ;;

    device-command)
      device_id=""
      capability=""
      command_name=""
      component="main"
      command_args=()
      while [[ $# -gt 0 ]]; do
        case "$1" in
          --device-id)
            device_id="$2"
            shift 2
            ;;
          --capability)
            capability="$2"
            shift 2
            ;;
          --command)
            command_name="$2"
            shift 2
            ;;
          --component)
            component="$2"
            shift 2
            ;;
          --arg)
            command_args+=("$2")
            shift 2
            ;;
          *)
            printf 'Unknown device-command option: %s\n' "$1" >&2
            exit 2
            ;;
        esac
      done
      if [[ -z "$device_id" || -z "$capability" || -z "$command_name" ]]; then
        st_json_error "device-command requires --device-id, --capability, and --command." | jq .
        exit 1
      fi
      if [[ ${#command_args[@]} -eq 0 ]]; then
        payload="$(jq -n \
          --arg component "$component" \
          --arg capability "$capability" \
          --arg command "$command_name" \
          '{
            commands: [
              {
                component: $component,
                capability: $capability,
                command: $command,
                arguments: []
              }
            ]
          }')"
      else
        output="$(printf '%s\n' "${command_args[@]}" | jq -R 'fromjson? // . ' | jq -s .)"
        payload="$(jq -n \
          --arg component "$component" \
          --arg capability "$capability" \
          --arg command "$command_name" \
          --argjson arguments "$output" \
          '{
            commands: [
              {
                component: $component,
                capability: $capability,
                command: $command,
                arguments: $arguments
              }
            ]
          }')"
      fi
      payload="$(st_api_request POST "/v1/devices/$device_id/commands" "$token" "$payload")" || {
        print_api_error "$payload"
        exit 1
      }
      st_attach_meta "$payload"
      ;;
  esac
}

main "$@"

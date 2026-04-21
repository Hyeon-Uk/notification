#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=./common.sh
source "$SCRIPT_DIR/common.sh"

usage() {
  cat <<'EOF'
Usage:
  setup_smartthings.sh [options]

Options:
  --config PATH
  --pat TOKEN
  --client-id ID
  --client-secret SECRET
  --scopes "r:devices:* x:devices:*"
  --redirect-uri URI
  --auth-base URL
  --auth-code CODE
  --force
  --help
EOF
}

main() {
  local config redirect_uri auth_base scopes auth_code force
  local pat client_id client_secret
  local token_payload access_token refresh_token expires_in token_scope auth_url

  st_require_cmd curl jq base64 date mktemp awk sed

  config="$(st_default_config)"
  redirect_uri="https://httpbin.org/get"
  auth_base="https://api.smartthings.com/oauth"
  auth_code=""
  force=0
  pat=""
  client_id=""
  client_secret=""
  scopes=""

  while [[ $# -gt 0 ]]; do
    case "$1" in
      --config)
        config="$2"
        shift 2
        ;;
      --pat)
        pat="$2"
        shift 2
        ;;
      --client-id)
        client_id="$2"
        shift 2
        ;;
      --client-secret)
        client_secret="$2"
        shift 2
        ;;
      --scopes)
        scopes="$2"
        shift 2
        ;;
      --redirect-uri)
        redirect_uri="$2"
        shift 2
        ;;
      --auth-base)
        auth_base="$2"
        shift 2
        ;;
      --auth-code)
        auth_code="$2"
        shift 2
        ;;
      --force)
        force=1
        shift
        ;;
      --help|-h)
        usage
        exit 0
        ;;
      *)
        printf 'Unknown option: %s\n' "$1" >&2
        usage >&2
        exit 2
        ;;
    esac
  done

  mkdir -p -- "$(dirname -- "$config")"
  [[ -f "$config" ]] || : > "$config"
  chmod 600 "$config" 2>/dev/null || true

  if [[ -n "$pat" ]]; then
    st_env_put SMARTTHINGS_PAT "$pat" "$config"
  fi
  if [[ -n "$client_id" ]]; then
    st_env_put SMARTTHINGS_CLIENT_ID "$client_id" "$config"
  fi
  if [[ -n "$client_secret" ]]; then
    st_env_put SMARTTHINGS_CLIENT_SECRET "$client_secret" "$config"
  fi
  if [[ -n "$scopes" ]]; then
    st_env_put SMARTTHINGS_SCOPES "$scopes" "$config"
  fi
  if [[ -n "$redirect_uri" ]]; then
    st_env_put SMARTTHINGS_REDIRECT_URI "$redirect_uri" "$config"
  fi

  auth_base="$(st_normalize_auth_base "$auth_base")"
  scopes="$(st_env_get SMARTTHINGS_SCOPES "$config")"
  [[ -n "$scopes" ]] || scopes="r:devices:* x:devices:*"
  st_env_put SMARTTHINGS_SCOPES "$scopes" "$config"

  redirect_uri="$(st_env_get SMARTTHINGS_REDIRECT_URI "$config")"
  [[ -n "$redirect_uri" ]] || redirect_uri="https://httpbin.org/get"
  st_env_put SMARTTHINGS_REDIRECT_URI "$redirect_uri" "$config"

  client_id="$(st_env_get SMARTTHINGS_CLIENT_ID "$config")"
  client_secret="$(st_env_get SMARTTHINGS_CLIENT_SECRET "$config")"

  if [[ -n "$auth_code" ]]; then
    if [[ -z "$client_id" || -z "$client_secret" ]]; then
      st_json_error "SMARTTHINGS_CLIENT_ID and SMARTTHINGS_CLIENT_SECRET are required before --auth-code." | jq .
      exit 1
    fi

    token_payload="$(
      st_http_form \
        "$auth_base/token" \
        "Authorization: Basic $(st_basic_auth_header "$client_id" "$client_secret")" \
        "grant_type=authorization_code" \
        "client_id=$client_id" \
        "redirect_uri=$redirect_uri" \
        "code=$auth_code"
    )" || {
      st_json_error "Auth-code exchange failed." | jq .
      exit 1
    }

    access_token="$(printf '%s' "$token_payload" | jq -r '.access_token // empty')"
    refresh_token="$(printf '%s' "$token_payload" | jq -r '.refresh_token // empty')"
    expires_in="$(printf '%s' "$token_payload" | jq -r '.expires_in // empty')"
    token_scope="$(printf '%s' "$token_payload" | jq -r '.scope // empty')"

    if [[ -z "$access_token" ]]; then
      printf '%s' "$token_payload" | jq .
      exit 1
    fi

    st_env_put SMARTTHINGS_ACCESS_TOKEN "$access_token" "$config"
    if [[ -n "$refresh_token" ]]; then
      st_env_put SMARTTHINGS_REFRESH_TOKEN "$refresh_token" "$config"
    fi
    if [[ -n "$expires_in" && "$expires_in" =~ ^[0-9]+$ ]]; then
      st_env_put SMARTTHINGS_TOKEN_EXPIRES_AT "$(( $(st_now_epoch) + expires_in ))" "$config"
    fi
    if [[ -n "$token_scope" ]]; then
      st_env_put SMARTTHINGS_TOKEN_SCOPE "$token_scope" "$config"
    fi

    jq -n --arg config "$config" '{
      ok: true,
      message: "SmartThings OAuth tokens saved.",
      config: $config
    }' | jq .
    exit 0
  fi

  if [[ -n "$(st_env_get SMARTTHINGS_PAT "$config")" || -n "$(st_env_get SMARTTHINGS_TOKEN "$config")" ]]; then
    jq -n --arg config "$config" '{
      ok: true,
      message: "PAT is configured for direct SmartThings API calls.",
      config: $config
    }' | jq .
    if [[ -z "$client_id" || -z "$client_secret" ]]; then
      exit 0
    fi
  fi

  if [[ -n "$client_id" && -n "$client_secret" ]]; then
    auth_url="$auth_base/authorize?client_id=$(st_urlencode "$client_id")&response_type=code&redirect_uri=$(st_urlencode "$redirect_uri")&scope=$(st_urlencode "$scopes")"
    jq -n \
      --arg config "$config" \
      --arg authorize_url "$auth_url" \
      --arg redirect_uri "$redirect_uri" \
      '{
        ok: true,
        message: "Open authorize_url on your phone, complete login, then rerun with --auth-code.",
        config: $config,
        redirect_uri: $redirect_uri,
        authorize_url: $authorize_url
      }' | jq .
    exit 0
  fi

  if [[ "$force" -eq 1 ]]; then
    jq -n --arg config "$config" '{
      ok: false,
      message: "Force requested, but this curl/jq setup cannot create a SmartThings OAuth app. Save PAT or existing OAuth client credentials first.",
      config: $config
    }' | jq .
    exit 1
  fi

  jq -n --arg config "$config" '{
    ok: false,
    message: "No usable SmartThings credentials found. Save SMARTTHINGS_PAT for direct API access, or provide SMARTTHINGS_CLIENT_ID and SMARTTHINGS_CLIENT_SECRET before running --auth-code.",
    config: $config
  }' | jq .
  exit 1
}

main "$@"

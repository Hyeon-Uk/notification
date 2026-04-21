#!/usr/bin/env bash
set -euo pipefail

st_skill_root() {
  cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd
}

st_default_config() {
  printf '%s/state/credentials.env\n' "$(st_skill_root)"
}

st_require_cmd() {
  local cmd
  for cmd in "$@"; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
      printf 'Missing required command: %s\n' "$cmd" >&2
      exit 1
    fi
  done
}

st_trim() {
  printf '%s' "$1" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//'
}

st_env_raw_value() {
  local key="$1"
  local file="$2"
  local line current found=""

  [[ -f "$file" ]] || return 0

  while IFS= read -r line || [[ -n "$line" ]]; do
    [[ "$line" =~ ^[[:space:]]*# ]] && continue
    [[ "$line" == *"="* ]] || continue
    current="${line%%=*}"
    current="$(st_trim "$current")"
    if [[ "$current" == "$key" ]]; then
      found="${line#*=}"
    fi
  done < "$file"

  printf '%s' "$found"
}

st_env_get() {
  local raw
  raw="$(st_env_raw_value "$1" "$2")"
  if [[ -z "$raw" ]]; then
    return 0
  fi
  printf '%s' "$raw" | jq -Rr 'fromjson? // .'
}

st_env_put() {
  local key="$1"
  local value="$2"
  local file="$3"
  local tmp encoded

  mkdir -p -- "$(dirname -- "$file")"
  [[ -f "$file" ]] || : > "$file"

  encoded="$(printf '%s' "$value" | jq -Rsa '.')"
  tmp="$(mktemp)"

  awk -v key="$key" -v value="$encoded" '
    BEGIN { updated = 0 }
    {
      if ($0 ~ "^[[:space:]]*" key "=") {
        if (!updated) {
          print key "=" value
          updated = 1
        }
        next
      }
      print
    }
    END {
      if (!updated) {
        print key "=" value
      }
    }
  ' "$file" > "$tmp"

  mv -- "$tmp" "$file"
  chmod 600 "$file" 2>/dev/null || true
}

st_now_epoch() {
  date +%s
}

st_json_error() {
  local message="$1"
  jq -n --arg error "$message" '{error:$error}'
}

st_format_json() {
  local payload="$1"
  printf '%s' "$payload" | jq .
}

st_normalize_auth_base() {
  local base="${1%/}"
  if [[ "$base" == */oauth ]]; then
    printf '%s\n' "$base"
  else
    printf '%s/oauth\n' "$base"
  fi
}

st_urlencode() {
  jq -nr --arg value "$1" '$value|@uri'
}

st_basic_auth_header() {
  printf '%s' "$1:$2" | base64 | tr -d '\n'
}

st_http_json() {
  local method="$1"
  local url="$2"
  local auth_header="$3"
  local payload="${4-}"
  local body_file http_code

  body_file="$(mktemp)"
  if [[ -n "$payload" ]]; then
    http_code="$(
      curl -sS -X "$method" "$url" \
        -H "Accept: application/json" \
        -H "$auth_header" \
        -H "Content-Type: application/json" \
        --data "$payload" \
        -o "$body_file" \
        -w '%{http_code}'
    )"
  else
    http_code="$(
      curl -sS -X "$method" "$url" \
        -H "Accept: application/json" \
        -H "$auth_header" \
        -o "$body_file" \
        -w '%{http_code}'
    )"
  fi

  ST_HTTP_STATUS="$http_code"
  cat -- "$body_file"
  rm -f -- "$body_file"
  [[ "$http_code" =~ ^2[0-9][0-9]$ ]]
}

st_http_form() {
  local url="$1"
  local auth_header="$2"
  shift 2

  local body_file http_code
  local -a args
  body_file="$(mktemp)"
  args=(
    -sS
    -X POST
    "$url"
    -H "Accept: application/json"
    -H "$auth_header"
    -H "Content-Type: application/x-www-form-urlencoded"
    -o "$body_file"
    -w '%{http_code}'
  )

  while [[ $# -gt 0 ]]; do
    args+=(--data-urlencode "$1")
    shift
  done

  http_code="$(curl "${args[@]}")"
  ST_HTTP_STATUS="$http_code"
  cat -- "$body_file"
  rm -f -- "$body_file"
  [[ "$http_code" =~ ^2[0-9][0-9]$ ]]
}

ST_TOKEN_EXPIRES_AT=""
ST_HTTP_STATUS=""

st_refresh_access_token() {
  local config="$1"
  local refresh_token client_id client_secret payload access_token new_refresh expires_in scope

  refresh_token="$(st_env_get SMARTTHINGS_REFRESH_TOKEN "$config")"
  client_id="$(st_env_get SMARTTHINGS_CLIENT_ID "$config")"
  client_secret="$(st_env_get SMARTTHINGS_CLIENT_SECRET "$config")"

  [[ -n "$refresh_token" && -n "$client_id" && -n "$client_secret" ]] || return 1

  payload="$(
    st_http_form \
      "https://api.smartthings.com/oauth/token" \
      "Authorization: Basic $(st_basic_auth_header "$client_id" "$client_secret")" \
      "grant_type=refresh_token" \
      "refresh_token=$refresh_token" \
      "client_id=$client_id"
  )" || {
    st_format_json "$(jq -n --arg body "$payload" --arg status "${ST_HTTP_STATUS:-}" '{
      error:{
        status: ($status | if . == "" then null else tonumber end),
        body:$body
      }
    }')" >&2
    return 1
  }

  access_token="$(printf '%s' "$payload" | jq -r '.access_token // empty')"
  [[ -n "$access_token" ]] || return 1

  st_env_put SMARTTHINGS_ACCESS_TOKEN "$access_token" "$config"

  new_refresh="$(printf '%s' "$payload" | jq -r '.refresh_token // empty')"
  if [[ -n "$new_refresh" ]]; then
    st_env_put SMARTTHINGS_REFRESH_TOKEN "$new_refresh" "$config"
  fi

  expires_in="$(printf '%s' "$payload" | jq -r '.expires_in // empty')"
  if [[ -n "$expires_in" ]]; then
    ST_TOKEN_EXPIRES_AT="$(( $(st_now_epoch) + expires_in ))"
    st_env_put SMARTTHINGS_TOKEN_EXPIRES_AT "$ST_TOKEN_EXPIRES_AT" "$config"
  else
    ST_TOKEN_EXPIRES_AT=""
  fi

  scope="$(printf '%s' "$payload" | jq -r '.scope // empty')"
  if [[ -n "$scope" ]]; then
    st_env_put SMARTTHINGS_TOKEN_SCOPE "$scope" "$config"
  fi

  printf '%s\n' "$access_token"
}

st_resolve_api_token() {
  local config="$1"
  local direct_token access_token expires_at

  direct_token="$(st_env_get SMARTTHINGS_PAT "$config")"
  if [[ -z "$direct_token" ]]; then
    direct_token="$(st_env_get SMARTTHINGS_TOKEN "$config")"
  fi
  if [[ -n "$direct_token" ]]; then
    ST_TOKEN_EXPIRES_AT=""
    printf '%s\n' "$direct_token"
    return 0
  fi

  access_token="$(st_env_get SMARTTHINGS_ACCESS_TOKEN "$config")"
  expires_at="$(st_env_get SMARTTHINGS_TOKEN_EXPIRES_AT "$config")"
  ST_TOKEN_EXPIRES_AT="$expires_at"

  if [[ -n "$access_token" ]]; then
    if [[ -n "$expires_at" ]] && [[ "$expires_at" =~ ^[0-9]+$ ]] && (( expires_at <= $(st_now_epoch) + 60 )); then
      access_token="$(st_refresh_access_token "$config")" || true
    fi
    if [[ -n "$access_token" ]]; then
      printf '%s\n' "$access_token"
      return 0
    fi
  fi

  printf '%s\n' ""
  return 1
}

st_api_request() {
  local method="$1"
  local path="$2"
  local token="$3"
  local payload="${4-}"

  st_http_json "$method" "https://api.smartthings.com$path" "Authorization: Bearer $token" "$payload"
}

st_attach_meta() {
  local payload="$1"

  if [[ -n "$ST_TOKEN_EXPIRES_AT" ]]; then
    printf '%s' "$payload" | jq --arg expires "$ST_TOKEN_EXPIRES_AT" '
      if type == "object" then
        ._meta.token_expires_at = $expires
      else
        .
      end
    '
  else
    st_format_json "$payload"
  fi
}

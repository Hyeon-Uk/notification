#!/bin/bash
# smoke_01_symbol_check.sh — verify all EXPORT_API symbols in libnotification.so

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LIB_PATH=""

# Search for the library
for candidate in \
  "${SCRIPT_DIR}/../../src/notification/libnotification.so" \
  "/usr/lib/libnotification.so" \
  "/usr/lib64/libnotification.so" \
  "$(find "${SCRIPT_DIR}/../.." -name 'libnotification.so' 2>/dev/null | head -1)"; do
  if [ -f "$candidate" ]; then
    LIB_PATH="$candidate"
    break
  fi
done

if [ -z "$LIB_PATH" ]; then
  echo "FAIL: libnotification.so not found"
  exit 1
fi

echo "Checking symbols in: $LIB_PATH"

REQUIRED_SYMBOLS=(
  notification_create
  notification_free
  notification_post
  notification_update
  notification_delete
  notification_delete_all
  notification_clone
  notification_set_image
  notification_get_image
  notification_set_text
  notification_get_text
  notification_set_sound
  notification_get_sound
  notification_set_vibration
  notification_get_vibration
  notification_set_led
  notification_get_led
  notification_set_property
  notification_get_property
  notification_set_display_applist
  notification_get_display_applist
  notification_set_size
  notification_get_size
  notification_set_progress
  notification_get_progress
  notification_set_layout
  notification_get_layout
  notification_get_type
  notification_set_tag
  notification_get_tag
  notification_load_by_tag
  notification_set_time
  notification_get_time
  notification_get_insert_time
  notification_set_launch_option
  notification_get_launch_option
  notification_set_event_handler
  notification_get_event_handler
  notification_set_led_time_period
  notification_get_led_time_period
  notification_set_text_domain
  notification_get_text_domain
  notification_set_time_to_text
  notification_get_time_from_text
  notification_get_pkgname
  notification_set_ongoing_flag
  notification_get_ongoing_flag
  notification_add_button
  notification_remove_button
  notification_set_auto_remove
  notification_get_auto_remove
  notification_save_as_template
  notification_create_from_template
  notification_get_noti_block_state
  notification_set_text_input
  notification_set_extension_image_size
  notification_get_extension_image_size
  notification_error_quark
  notification_list_get_head
  notification_list_get_tail
  notification_list_get_prev
  notification_list_get_next
  notification_list_get_data
  notification_list_get_count
  notification_list_append
  notification_list_remove
  notification_free_list
  notification_setting_get_setting_array
  notification_setting_get_setting_by_package_name
  notification_setting_get_package_name
  notification_setting_get_allow_to_notify
  notification_setting_set_allow_to_notify
  notification_setting_update_setting
  notification_setting_free_notification
  notification_status_monitor_message_cb_set
  notification_status_monitor_message_cb_unset
  notification_status_message_post
)

# DB API symbols
DB_SYMBOLS=(
  notification_db_init
  notification_db_open
  notification_db_close
  notification_db_exec
)

# IPC symbols
IPC_SYMBOLS=(
  notification_ipc_make_gvariant_from_noti
  notification_ipc_make_noti_from_gvariant
)

# Channel symbols
CHANNEL_SYMBOLS=(
  notification_channel_create
  notification_channel_free
  notification_channel_add
  notification_channel_remove
  notification_channel_update
  notification_channel_get_by_name
  notification_channel_set_blockable
  notification_channel_get_blockable
  notification_channel_set_block
  notification_channel_get_block
  notification_channel_get_name
  notification_channel_clone
  notification_channel_foreach
)

# Shared file symbols
SHARED_FILE_SYMBOLS=(
  notification_set_private_sharing
  notification_remove_private_sharing
  notification_validate_private_sharing
  notification_calibrate_private_sharing
)

# System settings symbols
SYSTEM_SETTING_SYMBOLS=(
  notification_system_setting_load_system_setting
  notification_system_setting_free_system_setting
  notification_system_setting_get_do_not_disturb
  notification_system_setting_set_do_not_disturb
)

# Ongoing symbols
ONGOING_SYMBOLS=(
  notification_ongoing_update_cb_set
  notification_ongoing_update_cb_unset
)

# Viewer symbols
VIEWER_SYMBOLS=(
  notification_init_default_viewer
  notification_launch_default_viewer
)

# UID variant symbols
UID_SYMBOLS=(
  notification_post_for_uid
  notification_update_for_uid
  notification_delete_for_uid
)

# Extension symbols
EXTENSION_SYMBOLS=(
  notification_set_extension_data
  notification_get_extension_data
)

# Checkbox symbols
CHECKBOX_SYMBOLS=(
  notification_set_check_box
  notification_get_check_box
)

# Deprecated compat symbols
DEPRECATED_SYMBOLS=(
  notification_insert
  notification_new
  notification_load
)

# TIDL symbols
TIDL_SYMBOLS=(
)

# Noti DB symbols
NOTI_DB_SYMBOLS=(
  notification_noti_insert
  notification_noti_update
  notification_noti_delete_all
  notification_noti_get_by_priv_id
)

# Helper: check a list of symbols, print MISSING for each absent one, return count of missing
check_symbols() {
  local group_name="$1"
  shift
  local syms=("$@")
  local missing=0
  for sym in "${syms[@]}"; do
    if ! nm -D "$LIB_PATH" 2>/dev/null | grep -q " T ${sym}$\| T ${sym}@@"; then
      echo "MISSING [${group_name}]: $sym" >&2
      missing=$((missing + 1))
    fi
  done
  echo "$missing"
}

TOTAL_MISSING=0

# --- Core symbols ---
CORE_MISSING=0
for sym in "${REQUIRED_SYMBOLS[@]}"; do
  if ! nm -D "$LIB_PATH" 2>/dev/null | grep -q " T ${sym}$\| T ${sym}@@"; then
    echo "MISSING [core]: $sym"
    CORE_MISSING=$((CORE_MISSING + 1))
  fi
done
TOTAL_MISSING=$((TOTAL_MISSING + CORE_MISSING))

# --- DB API ---
RES=$(check_symbols "db" "${DB_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- IPC ---
RES=$(check_symbols "ipc" "${IPC_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Channel ---
RES=$(check_symbols "channel" "${CHANNEL_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Shared file ---
RES=$(check_symbols "shared_file" "${SHARED_FILE_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- System settings ---
RES=$(check_symbols "system_setting" "${SYSTEM_SETTING_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Ongoing ---
RES=$(check_symbols "ongoing" "${ONGOING_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Viewer ---
RES=$(check_symbols "viewer" "${VIEWER_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- UID variants ---
RES=$(check_symbols "uid" "${UID_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Extension ---
RES=$(check_symbols "extension" "${EXTENSION_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Checkbox ---
RES=$(check_symbols "checkbox" "${CHECKBOX_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Deprecated compat ---
RES=$(check_symbols "deprecated_compat" "${DEPRECATED_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- TIDL ---
RES=$(check_symbols "tidl" "${TIDL_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- Noti DB ---
RES=$(check_symbols "noti_db" "${NOTI_DB_SYMBOLS[@]}")
TOTAL_MISSING=$((TOTAL_MISSING + RES))

# --- SYM_50: negative check — __notification_mutex_lock must NOT be exported ---
NEGATIVE_SYM="__notification_mutex_lock"
if nm -D "$LIB_PATH" 2>/dev/null | grep -q " T ${NEGATIVE_SYM}$\| T ${NEGATIVE_SYM}@@"; then
  echo "FAIL [SYM_50]: Internal symbol '${NEGATIVE_SYM}' is exported (should be static)"
  TOTAL_MISSING=$((TOTAL_MISSING + 1))
else
  echo "PASS [SYM_50]: '${NEGATIVE_SYM}' is correctly not exported"
fi

TOTAL_CHECKED=$(( ${#REQUIRED_SYMBOLS[@]} + ${#DB_SYMBOLS[@]} + ${#IPC_SYMBOLS[@]} + \
  ${#CHANNEL_SYMBOLS[@]} + ${#SHARED_FILE_SYMBOLS[@]} + ${#SYSTEM_SETTING_SYMBOLS[@]} + \
  ${#ONGOING_SYMBOLS[@]} + ${#VIEWER_SYMBOLS[@]} + ${#UID_SYMBOLS[@]} + \
  ${#EXTENSION_SYMBOLS[@]} + ${#CHECKBOX_SYMBOLS[@]} + ${#DEPRECATED_SYMBOLS[@]} + \
  ${#TIDL_SYMBOLS[@]} + ${#NOTI_DB_SYMBOLS[@]} ))

if [ "$TOTAL_MISSING" -gt 0 ]; then
  echo "FAIL: $TOTAL_MISSING symbols missing out of $TOTAL_CHECKED checked"
  exit 1
fi

echo "PASS: All ${TOTAL_CHECKED} symbols found (+ negative check passed)"
exit 0

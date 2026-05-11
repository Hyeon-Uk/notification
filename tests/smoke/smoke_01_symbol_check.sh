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

MISSING=0
for sym in "${REQUIRED_SYMBOLS[@]}"; do
  if ! nm -D "$LIB_PATH" | grep -q " T ${sym}$\| T ${sym}@@"; then
    echo "MISSING: $sym"
    MISSING=$((MISSING + 1))
  fi
done

if [ $MISSING -gt 0 ]; then
  echo "FAIL: $MISSING symbols missing"
  exit 1
fi

echo "PASS: All ${#REQUIRED_SYMBOLS[@]} symbols found"
exit 0

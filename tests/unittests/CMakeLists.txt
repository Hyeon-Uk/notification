AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src UNITTESTS_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../mock MOCK_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/src NOTIFICATION_PROD_SRCS)

# notification.c and friends are compiled directly into the unittest binary
# (instead of linking libnotification.so) so that hidden-visibility internal
# calls, e.g. notification_tidl.c -> notification_tidl_proxy.c, resolve
# within this single link and can be intercepted with -Wl,--wrap below.
ADD_EXECUTABLE(${TARGET_NOTIFICATION_UNITTESTS}
  ${UNITTESTS_SRCS}
  ${MOCK_SRCS}
  ${NOTIFICATION_PROD_SRCS}
)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_UNITTESTS} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification-ex
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification-ex/api
  ${CMAKE_CURRENT_SOURCE_DIR}/../mock
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_UNITTESTS} PUBLIC
  AUL_DEPS
  BUNDLE_DEPS
  CAPI_APPFW_APP_COMMON_DEPS
  CAPI_APPFW_APP_CONTROL_DEPS
  CAPI_APPFW_PACKAGE_MANAGER_DEPS
  CAPI_SYSTEM_INFO_DEPS
  CAPI_SYSTEM_RESOURCE_DEPS
  DLOG_DEPS
  GIO_DEPS
  GIO_UNIX_DEPS
  GLIB_DEPS
  GMOCK_DEPS
  INIPARSER_DEPS
  LIBTZPLATFORM_CONFIG_DEPS
  PKGMGR_INFO_DEPS
  RPC_PORT_DEPS
  SECURITY_MANAGER_DEPS
  TIZEN_CORE_DEPS
  VCONF_DEPS
)

TARGET_LINK_LIBRARIES(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  ${TARGET_NOTIFICATION_EX} "-ldl")

# rpc_port_proxy_* functions have hidden ELF visibility (project-wide
# -fvisibility=hidden) and are only reachable via -Wl,--wrap since
# notification_tidl_proxy.c is now compiled directly into this binary
# alongside its callers instead of via libnotification.so.
SET(NOTI_SERVICE_PROXY_WRAP_SYMBOLS
  rpc_port_proxy_list_notification_destroy
  rpc_port_proxy_list_notification_foreach
  rpc_port_proxy_noti_service_changed_create
  rpc_port_proxy_noti_service_changed_destroy
  rpc_port_proxy_noti_service_changed_dispose
  rpc_port_proxy_noti_service_changed_set_callback
  rpc_port_proxy_noti_service_connect_sync
  rpc_port_proxy_noti_service_create
  rpc_port_proxy_noti_service_destroy
  rpc_port_proxy_noti_service_event_received_create
  rpc_port_proxy_noti_service_event_received_destroy
  rpc_port_proxy_noti_service_event_received_dispose
  rpc_port_proxy_noti_service_event_received_set_callback
  rpc_port_proxy_noti_service_invoke_add_noti
  rpc_port_proxy_noti_service_invoke_channel_delete
  rpc_port_proxy_noti_service_invoke_channel_get
  rpc_port_proxy_noti_service_invoke_channel_get_list
  rpc_port_proxy_noti_service_invoke_channel_insert
  rpc_port_proxy_noti_service_invoke_channel_update
  rpc_port_proxy_noti_service_invoke_check_event_receiver
  rpc_port_proxy_noti_service_invoke_create_from_package_template
  rpc_port_proxy_noti_service_invoke_create_from_template
  rpc_port_proxy_noti_service_invoke_delete_multiple_noti
  rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist
  rpc_port_proxy_noti_service_invoke_delete_single_noti
  rpc_port_proxy_noti_service_invoke_get_all_noti_count
  rpc_port_proxy_noti_service_invoke_get_noti_block_state
  rpc_port_proxy_noti_service_invoke_get_noti_count
  rpc_port_proxy_noti_service_invoke_get_setting_array
  rpc_port_proxy_noti_service_invoke_get_setting_by_app_id
  rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id
  rpc_port_proxy_noti_service_invoke_load_noti_by_tag
  rpc_port_proxy_noti_service_invoke_load_noti_detail_list
  rpc_port_proxy_noti_service_invoke_load_noti_grouping_list
  rpc_port_proxy_noti_service_invoke_load_system_setting
  rpc_port_proxy_noti_service_invoke_refresh_noti
  rpc_port_proxy_noti_service_invoke_register_changed
  rpc_port_proxy_noti_service_invoke_register_dnd_app
  rpc_port_proxy_noti_service_invoke_register_event
  rpc_port_proxy_noti_service_invoke_register_service
  rpc_port_proxy_noti_service_invoke_reset_event_handler
  rpc_port_proxy_noti_service_invoke_save_as_template
  rpc_port_proxy_noti_service_invoke_send_event
  rpc_port_proxy_noti_service_invoke_send_event_by_priv_id
  rpc_port_proxy_noti_service_invoke_unregister_changed
  rpc_port_proxy_noti_service_invoke_unregister_dnd_app
  rpc_port_proxy_noti_service_invoke_unregister_event
  rpc_port_proxy_noti_service_invoke_update_dnd_allow_exception
  rpc_port_proxy_noti_service_invoke_update_noti
  rpc_port_proxy_noti_service_invoke_update_noti_setting
  rpc_port_proxy_noti_service_invoke_update_noti_sys_setting
)

SET(NOTI_SERVICE_PROXY_WRAP_FLAGS "")
FOREACH(_wrap_sym ${NOTI_SERVICE_PROXY_WRAP_SYMBOLS})
  SET(NOTI_SERVICE_PROXY_WRAP_FLAGS
    "${NOTI_SERVICE_PROXY_WRAP_FLAGS} -Wl,--wrap=${_wrap_sym}")
ENDFOREACH()

SET_TARGET_PROPERTIES(${TARGET_NOTIFICATION_UNITTESTS} PROPERTIES
  LINK_FLAGS "${NOTI_SERVICE_PROXY_WRAP_FLAGS}")

INSTALL(TARGETS ${TARGET_NOTIFICATION_UNITTESTS} DESTINATION bin)

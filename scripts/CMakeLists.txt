CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/505.notification_upgrade.sh.in
  505.notification_upgrade.sh @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/505.notification_upgrade.sh
  DESTINATION /usr/share/upgrade/scripts/)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/505.notification_upgrade.sh.in
  upgrade.sh @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/upgrade.sh
  DESTINATION ${SYSCONF_INSTALL_DIR}/notification/)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/11_notification-add.post
  DESTINATION ${SYSCONF_INSTALL_DIR}/gumd/useradd.d/)

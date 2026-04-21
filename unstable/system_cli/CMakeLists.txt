# Install index.md
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/index.md
        DESTINATION ${APP_DATA_DIR}/tizen-tools/system_cli/
        COMPONENT system_cli)

# Install filter script
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/filter_system_cli.sh
        DESTINATION ${APP_DATA_DIR}/tizen-tools/system_cli/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        COMPONENT system_cli)

# Install all subdirectories with markdown documentation
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/accessibility
        ${CMAKE_CURRENT_SOURCE_DIR}/action_framework
        ${CMAKE_CURRENT_SOURCE_DIR}/application
        ${CMAKE_CURRENT_SOURCE_DIR}/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/compression
        ${CMAKE_CURRENT_SOURCE_DIR}/configuration
        ${CMAKE_CURRENT_SOURCE_DIR}/database
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus
        ${CMAKE_CURRENT_SOURCE_DIR}/device
        ${CMAKE_CURRENT_SOURCE_DIR}/device_mapper
        ${CMAKE_CURRENT_SOURCE_DIR}/display
        ${CMAKE_CURRENT_SOURCE_DIR}/dotnet
        ${CMAKE_CURRENT_SOURCE_DIR}/filesystem
        ${CMAKE_CURRENT_SOURCE_DIR}/gstreamer
        ${CMAKE_CURRENT_SOURCE_DIR}/json
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel_module
        ${CMAKE_CURRENT_SOURCE_DIR}/locale
        ${CMAKE_CURRENT_SOURCE_DIR}/logging
        ${CMAKE_CURRENT_SOURCE_DIR}/network
        ${CMAKE_CURRENT_SOURCE_DIR}/package_management
        ${CMAKE_CURRENT_SOURCE_DIR}/resource
        ${CMAKE_CURRENT_SOURCE_DIR}/security
        ${CMAKE_CURRENT_SOURCE_DIR}/service
        ${CMAKE_CURRENT_SOURCE_DIR}/system
        ${CMAKE_CURRENT_SOURCE_DIR}/text_processing
        ${CMAKE_CURRENT_SOURCE_DIR}/tizen_platform
        DESTINATION ${APP_DATA_DIR}/tizen-tools/system_cli/
        COMPONENT system_cli
        FILES_MATCHING
        PATTERN "*.md"
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "CMakeFiles" EXCLUDE
        PATTERN "*.cmake" EXCLUDE
        PATTERN "build" EXCLUDE
        PATTERN "CMakeCache.txt" EXCLUDE)

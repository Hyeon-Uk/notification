CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(notification-ex_unittests CXX)

INCLUDE(FindPkgConfig)
pkg_check_modules(notification-ex_unittests REQUIRED
    dlog
    gmock
    capi-appfw-app-control
    glib-2.0
    aul
    security-manager
    libtzplatform-config
)

FOREACH(flag ${notification-ex_unittests_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Werror")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../notification-ex)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../notification-ex/api)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../unittest/mock)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/mock UNIT_TEST_SOURCES)
AUX_SOURCE_DIRECTORY(src SOURCES)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../notification-ex NOTIFICATION_EX_SOURCES)
ADD_EXECUTABLE(${PROJECT_NAME}
    ${SOURCES}
    ${NOTIFICATION_EX_SOURCES}
    ${UNIT_TEST_SOURCES}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${notification-ex_unittests_LDFLAGS}
    ${pkgs_LDFLAGS}
    ${pkgs_LIBRARIES}
    gmock
    notification-ex
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)

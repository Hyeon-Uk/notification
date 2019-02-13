CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(notification-ex_unittests CXX)

INCLUDE(FindPkgConfig)
pkg_check_modules(notification-ex_unittests REQUIRED
    dlog
    gmock
    capi-appfw-app-control
)

FOREACH(flag ${notification-ex_unittests_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Werror -Winline")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../notification-ex)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

AUX_SOURCE_DIRECTORY(src SOURCES)
ADD_EXECUTABLE(${PROJECT_NAME}
    ${SOURCES}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${notification-ex_unittests_LDFLAGS}
    ${pkgs_LDFLAGS}
    ${pkgs_LIBRARIES}
    gmock
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)

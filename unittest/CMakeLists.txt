CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(notification_ex_unittests CXX)

INCLUDE(FindPkgConfig)
pkg_check_modules(notification_ex_unittests REQUIRED
    dlog
    gmock
)

FOREACH(flag ${notification_ex_unittests_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Werror -Winline")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

AUX_SOURCE_DIRECTORY(src SOURCES)
ADD_EXECUTABLE(${PROJECT_NAME}
    ${SOURCES}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${watchface-complication_unittests_LDFLAGS}
    ${pkgs_LDFLAGS}
    ${pkgs_LIBRARIES}
    gmock
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-private-field")
endif()

ADD_SUBDIRECTORY(notification)
ADD_SUBDIRECTORY(notification-ex)
ADD_SUBDIRECTORY(notification-init)
ADD_SUBDIRECTORY(notification-test-app)

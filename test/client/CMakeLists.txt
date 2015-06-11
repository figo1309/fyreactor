cmake_minimum_required(VERSION 2.6)
MESSAGE("main dir:" ${PROJECT_SOURCE_DIR})

#if (NOT CMAKE_BUILD_TYPE STREQUAL 'DEBUG')
#	add_definitions(-std=c++11 -gstabs+ ${CMAKE_OPTION})
#	set(CMAKE_EXE_LINKER_FLAGS "-g  -Wall -lpthread -O0")
#else()
#    add_definitions(-DHAVE_EPOLL -g -w -gstabs -std=c++11 ${CMAKE_OPTION})
#    set(CMAKE_EXE_LINKER_FLAGS "-g -w -gstabs -Wall -lpthread")
#endif()

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../include/)

ADD_LIBRARY(fyreactor SHARED IMPORTED)
SET_TARGET_PROPERTIES(fyreactor PROPERTIES IMPORTED_LOCATION
    "${PROJECT_SOURCE_DIR}/../../lib64/linux/libfyreactor.a")

add_definitions(-std=c++11 -g -gdwarf-2 -DHAVE_EPOLL ${CMAKE_OPTION})
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
aux_source_directory(. SOURCES)
message("TCPClient SOURCES = " ${SOURCES})

add_executable(TCPClient ${SOURCES})

TARGET_LINK_LIBRARIES(TCPClient ${CMAKE_EXE_LINKER_FLAGS})
TARGET_LINK_LIBRARIES(TCPClient fyreactor)
TARGET_LINK_LIBRARIES(TCPClient ${CMAKE_EXE_LINKER_FLAGS} ${LOG_SYS})



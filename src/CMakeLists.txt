
aux_source_directory(./net NET_SRCS)
aux_source_directory(./util UTIL_SRCS)
aux_source_directory(./xml XML_SRCS)
set(SOURCES ${UTIL_SRCS} ${NET_SRCS} ${XML_SRCS})
ADD_DEFINITIONS(-std=c++11 -g -gdwarf-2 -W -Wall)
IF(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
	add_library(fyreactor32 STATIC ${SOURCES})
ELSE()
	add_library(fyreactor STATIC ${SOURCES})
ENDIF()


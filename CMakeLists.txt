cmake_minimum_required(VERSION 2.8)

project(fyreactor)

#set(CMAKE_C_COMPILER				"clang")
#set(CMAKE_C_FLAGS					"-Wall -std=c99")
#set(CMAKE_C_FLAGS_DEBUG				"-g")
#set(CMAKE_C_FLAGS_MINSIZEREL		"-Os -DNDEBUG")
#set(CMAKE_C_FLAGS_RELEASE			"-O4 -DNDEBUG")
#set(CMAKE_C_FLAGS_RELWITHDEBINFO	"-O0 -g")

set(CMAKE_CXX_COMPILER				"g++")
set(CMAKE_CXX_FLAGS					"-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG			"-g")
#set(CMAKE_CXX_FLAGS_MINSIZEREL		"-Os -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE			"-O4 -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"-O0 -g")

message(" *PROJECT_SOURCE_DIR = " ${PROJECT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE MATCHES "DEBUG")
	set( CMAKE_BUILD_TYPE DEBUG)
	message(" *ENABLE " ${CMAKE_BUILD_TYPE} " MODE")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
	set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME})
else()
	set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib64/${CMAKE_SYSTEM_NAME})
endif()


message(" *LIB_DIR =" ${LIB_DIR} )

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
message(" *SRC_DIR =" ${SRC_DIR} )

set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
message(" *BIN_DIR =" ${BIN_DIR} )

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
message(" *INCLUDE_DIR = " ${INCLUDE_DIR})

include_directories(  
	${fyreactor_SOURCE_DIR}/include
	${fyreactor_SOURCE_DIR}/
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
	link_directories(
		${fyreactor_SOURCE_DIR}/lib
	)
else()
	link_directories(  
		${fyreactor_SOURCE_DIR}/lib64
	)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fstack-protector -fstack-protector-all -lpthread -lz")

set(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})

set(LIBRARY_OUTPUT_PATH ${LIB_DIR})

set(CMAKE_OPTION "${CMAKE_OPTION}")

ADD_SUBDIRECTORY(src)



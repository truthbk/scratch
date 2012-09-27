cmake_minimum_required (VERSION 2.6)
project (mkload)

# version
set (mkload_VERSION_MAJOR 0)
set (mkload_VERSION_MINOR 1)
set (mkload_VERSION_PATCH 0)


#should we bundle the libs??? 

# Locate required packages
# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include_directories(${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)

#should check for linux explicitly...
if(APPLE)
	add_definitions("-D_OSX")
else()
	add_definitions("-D_LINUX")
endif()

#compiler stuff
set(CMAKE_C_FLAGS "-ggdb -Wall -W -fbuiltin -lm")
set(CMAKE_CXX_FLAGS "-ggdb -Wall -W -fbuiltin -pthread")
# compile as C++0x
# add_definitions(-std=c++0x -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H)
add_subdirectory(src)

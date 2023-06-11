cmake_minimum_required(VERSION 2.8.12)

if(${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

project (HFWorkshop)

# CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Directory variables
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SWF_DIR ${LIB_DIR}/libswf/libswf)
set(SWF_LIB_DIR ${LIB_DIR}/libswf/libswf/libraries)
set(SWF_INC_DIR ${LIB_DIR}/libswf/libswf/include)
set(SWF_SRC_DIR ${LIB_DIR}/libswf/libswf/source)

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCC OR CMAKE_COMPILER_IS_CLANGCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	endif ()
else ()
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

### DEPENDENCIES

# https://cmake.org/cmake/help/v3.0/module/FindZLIB.html
find_package(ZLIB REQUIRED)

# Gettext executables
# https://cmake.org/cmake/help/v3.0/module/FindGettext.html
find_package(Gettext REQUIRED)

# Libintl required by gettext
find_package(Intl REQUIRED)

# GNU Readline (not in use)
#find_package(Readline REQUIRED)

# Editline
find_package(Editline REQUIRED)

# Minizip
find_package(MiniZip REQUIRED)

if( NOT EXISTS ${SWF_DIR}/Makefile )
	message(FATAL_ERROR "Unable to find libswf.")
endif()
if( NOT EXISTS ${SWF_LIB_DIR}/lzma/Makefile )
	message(FATAL_ERROR "Unable to find lzma sdk.")
endif()
if( NOT EXISTS ${SWF_LIB_DIR}/lodepng/lodepng/Makefile )
	message(FATAL_ERROR "Unable to find lodepng library.")
endif()

# Set Warnings and linking options
if(MSVC)
	# For static linking
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	# Disable secure warnings
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	# No warnings for compiling external libraries
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCC OR CMAKE_COMPILER_IS_CLANGCXX)
	# No warnings for compiling external libraries
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

if(WIN32)
	add_compile_definitions(UNICODE _UNICODE)
endif(WIN32)

### BUILD LIBRARIES ###
# (Generate the static library from the sources)

# Compile LZMA SDK
add_compile_definitions(_7ZIP_ST)
add_library(lzmasdk STATIC ${SWF_LIB_DIR}/lzma/C/LzmaEnc.c ${SWF_LIB_DIR}/lzma/C/LzFind.c
	${SWF_LIB_DIR}/lzma/C/LzmaDec.c ${SWF_LIB_DIR}/lzma/C/Lzma2Dec.c)

# Compile LodePNG
add_compile_definitions(DLODEPNG_NO_COMPILE_ZLIB LODEPNG_NO_COMPILE_DISK)
add_library(lodepng STATIC ${SWF_LIB_DIR}/lodepng/lodepng/lodepng.cpp)

# Compile libswf
include_directories( SYSTEM ${SWF_LIB_DIR}/lodepng )
file(GLOB LIBSWF_SRC_FILES "${SWF_SRC_DIR}/*.cpp")
add_library(swf STATIC ${LIBSWF_SRC_FILES})


### BUILD PROGRAM ###

# Set Warnings and linking options
if(MSVC)
	# For static linking
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	# Disable secure warnings
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCC OR CMAKE_COMPILER_IS_CLANGCXX)
	# Update if necessary
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wmain -Weffc++ -Wswitch-default \
		-Wswitch-enum -Wmissing-include-dirs -Wmissing-declarations -Wunreachable-code -Winline \
		-Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow -Wnon-virtual-dtor \
		-Wconversion -Wstrict-aliasing -Wold-style-cast")
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wzero-as-null-pointer-constant") # -Wuseless-cast
	endif()
endif()

## Libraries - Part 1

#cBring the headers into the project
include_directories( SYSTEM ${INC_DIR} )
include_directories( SYSTEM ${ZLIB_INCLUDE_DIRS} ) # From find_package(zlib)

# GNU Readline (not in use)
#if(WIN32)
#	add_compile_definitions(USE_READLINE_STATIC)
#endif(WIN32)
#include_directories( SYSTEM ${Readline_INCLUDE_DIR} ) # From find_package(Readline)

include_directories( SYSTEM ${Editline_INCLUDE_DIR} ) # From find_package(Editline)
include_directories( SYSTEM ${Intl_INCLUDE_DIRS} ) # From find_package(Libintl)
include_directories( SYSTEM ${MINIZIP_INCLUDE_DIR} ) # From find_package(MiniZip)
include_directories( SYSTEM ${LIB_DIR}/libswf/libswf/libraries )
include_directories( SYSTEM ${LIB_DIR}/libswf/libswf/source )
include_directories( SYSTEM ${LIB_DIR}/libswf/libswf/include )
include_directories( SYSTEM ${LIB_DIR} ) # rlutil

# The file(GLOB...) allows for wildcard additions:
file(GLOB SRC_FILES "${SOURCE_DIR}/*.cpp")

if (WIN32)
	file(GLOB RES_FILES "${RESOURCE_DIR}/*.rc")
	list(APPEND SRC_FILES ${RES_FILES})
endif (WIN32)

add_executable(HFWorkshop ${SRC_FILES})

## Libraries - Part 2
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCC OR CMAKE_COMPILER_IS_CLANGCXX)
	target_link_libraries( HFWorkshop z )
endif()
target_link_libraries( HFWorkshop ${ZLIB_LIBRARIES} ) # From find_package(zlib)

# GNU Readline (not in use)
#target_link_libraries( HFWorkshop ${Readline_LIBRARY} ) # From find_package(Readline)
#if(WIN32 AND NOT MSVC)
#	target_link_libraries(HFWorkshop termcap) # Readline
#endif()

target_link_libraries( HFWorkshop ${Editline_LIBRARY} ) # From find_package(Editline)
target_link_libraries( HFWorkshop ${Intl_LIBRARIES} ) # From find_package(Intl)
target_link_libraries( HFWorkshop ${MINIZIP_LIBRARIES} ) # From find_package(MiniZip)
target_link_libraries(HFWorkshop swf) # libswf
target_link_libraries(HFWorkshop lzmasdk) # LZMA SDK
#target_link_libraries(HFWorkshop lzma) # XZ Utils
target_link_libraries(HFWorkshop lodepng) # LodePNG

if( MINGW )
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

## Install
install (TARGETS HFWorkshop DESTINATION HFWorkshop)

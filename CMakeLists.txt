cmake_minimum_required (VERSION 2.6)

# Guess system architecture
set(GUESS_ARCH ${CMAKE_SYSTEM_PROCESSOR})
if(CMAKE_OSX_ARCHITECTURES)
	set(GUESS_ARCH ${CMAKE_OSX_ARCHITECTURES})
endif()
# We don't do any differentiation for AMD64 instruction set
if(${GUESS_ARCH} STREQUAL "AMD64")
	set(GUESS_ARCH "x86_64")
endif()

# Prompt architecture
set(TARGET_ARCH "${GUESS_ARCH}" CACHE STRING "Define the target architecture" )
set(CMAKE_OSX_ARCHITECTURES ${TARGET_ARCH})
set(OSX_ARCHITECTURES ${TARGET_ARCH})
message(STATUS "Building cernvm-cli for arch: ${TARGET_ARCH}")

# CernVM Library
project ( cernvm-cli )
set(VERSION "1.0.0")

# Include additional libraries
include(cmake/AddCompileLinkFlags.cmake)

#############################################################
# CERNVM LIBRARY 
#############################################################

# CernVM Library sources
set( CERNVM_LIBSRC "../libcernvm" )
set( USE_SYSTEM_LIBS 0 )

# Include sub-project
add_subdirectory( ${CERNVM_LIBSRC} libcernvm )

#############################################################
# SOURCES 
#############################################################

# Fixes for windows
if (WIN32)
	# Fix: Disable Min/Max macros
	ADD_DEFINITIONS(-DNOMINMAX)   
	# Fix: For some reason CURL_STATICLIB is not defined
	ADD_DEFINITIONS(-DCURL_STATICLIB)
endif(WIN32)

# Locate the common sources
file ( GLOB CERNVM_CLI_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp  )

# Setup includes
include_directories( ${PROJECT_SOURCE_DIR}/src )
include_directories( ${CERNVM_INCLUDE_DIRS} )

# Add executable
add_executable( ${PROJECT_NAME} 
	${CERNVM_CLI_SOURCES}
	)

# Post-target Fixes for windows
if (WIN32)
	# Fix: OpenSSL builds are not built with safe exception handlers
	my_add_link_flags( ${PROJECT_NAME} "/SAFESEH:NO")
endif(WIN32)

# Libraries
target_link_libraries ( ${PROJECT_NAME} ${CERNVM_LIBRARIES} )

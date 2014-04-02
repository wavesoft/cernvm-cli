cmake_minimum_required (VERSION 2.6)

# CernVM Library
project ( cernvm-cli )
set(VERSION "1.0.0")

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

# Windows additional definitions
if (WIN32)
	ADD_DEFINITIONS(-DNOMINMAX)   
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

# Libraries
target_link_libraries ( ${PROJECT_NAME} ${CERNVM_LIBRARIES} )

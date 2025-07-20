cmake_minimum_required( VERSION 3.5 )

set( PROJECT_NAME wfcompress )
project( ${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

file( GLOB SOURCEFILES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp" )

add_executable( ${PROJECT_NAME} ${SOURCEFILES} )

if (WIN32)
    target_link_libraries( ${PROJECT_NAME} -static )
endif (WIN32)

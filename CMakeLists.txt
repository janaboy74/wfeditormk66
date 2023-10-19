cmake_minimum_required(VERSION 3.10)
project(wfeditormk66 CXX)
set(PROJECT_TARGET wfeditormk66)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED "gtkmm-3.0")

file( GLOB SRCS
    "${PROJECT_SOURCE_DIR}/src/*.cpp" )

add_executable(${PROJECT_TARGET} ${SRCS})
target_link_libraries(${PROJECT_TARGET} PUBLIC ${GTK_LIBRARIES})
target_include_directories(${PROJECT_TARGET} PUBLIC ${GTK_INCLUDE_DIRS})

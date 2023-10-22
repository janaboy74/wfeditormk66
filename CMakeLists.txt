cmake_minimum_required(VERSION 3.10)
project(wfeditormk66 CXX)
set(PROJECT_TARGET wfeditormk66)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED "gtkmm-3.0")

file( GLOB SRCS
    "${PROJECT_SOURCE_DIR}/src/*.cpp" )

string(FIND ${CMAKE_SYSTEM_NAME} "Linux" OS_NAME)
if(${OS_NAME} GREATER_EQUAL 0)
  set(EXTRA_LINKER_FLAGS "")
else()
  set(EXTRA_LINKER_FLAGS "-lgcc -lstdc++ -mwindows")
endif()

add_executable(${PROJECT_TARGET} ${SRCS})
target_link_libraries(${PROJECT_TARGET} PUBLIC ${GTKMM_LIBRARIES} ${EXTRA_LINKER_FLAGS})
target_include_directories(${PROJECT_TARGET} PUBLIC ${GTKMM_INCLUDE_DIRS})
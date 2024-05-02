cmake_minimum_required(VERSION 3.5)
project(lcpp)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")
include_directories(${CMAKE_CURRENT_LIST_DIR})

aux_source_directory(. SRC_LIST)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR})
IF (WIN32)
    link_directories(
        ${CMAKE_CURRENT_LIST_DIR}
    )
ENDIF()

add_executable(lcpp ${SRC_LIST})
IF (WIN32)
    target_link_libraries(lcpp lua)
ELSE ()
    target_link_libraries(lcpp lua dl)
ENDIF ()

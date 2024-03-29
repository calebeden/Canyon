cmake_minimum_required(VERSION 3.14)

# Contains inspiration from https://github.com/selyunin/gtest_submodule/blob/master/CMakeLists.txt
# And https://github.com/codetechandtutorials/OurLordAndSavior/

# #######################################
# Project setup
# #######################################
project(canyon VERSION 0.0.1)
set(PROJECT_EXECUTABLE ${PROJECT_NAME})
set(PROJECT_LIBRARY ${PROJECT_NAME}_lib)
set(PROJECT_TESTS ${PROJECT_NAME}_test)

# Compiler flags
option(DEBUG "compile with debugging symbols" OFF)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "-g -Wall -Werror -Wextra -Wpedantic")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
endif()

set(CMAKE_CXX_STANDARD 17)

# Define include directories
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

# #######################################
# Source files
# #######################################
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Main - separate executable so that tests can have own main
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)

# #######################################
# Compile source files into a library
# #######################################
add_library(${PROJECT_LIBRARY} ${SRC_FILES})

# #######################################
# Main is separate (e.g. library client)
# #######################################
add_executable(${PROJECT_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/main.cpp)

# #######################################
# Configuration file
# #######################################
# Set default version if not defined
if(NOT DEFINED ${PROJECT_NAME}_VERSION_MAJOR OR ${PROJECT_NAME}_VERSION_MAJOR STREQUAL "")
    set(${PROJECT_NAME}_VERSION_MAJOR 0)
endif()

if(NOT DEFINED ${PROJECT_NAME}_VERSION_MINOR OR ${PROJECT_NAME}_VERSION_MINOR STREQUAL "")
    set(${PROJECT_NAME}_VERSION_MINOR 0)
endif()

if(NOT DEFINED ${PROJECT_NAME}_VERSION_PATCH OR ${PROJECT_NAME}_VERSION_PATCH STREQUAL "")
    set(${PROJECT_NAME}_VERSION_PATCH 0)
endif()

message(STATUS "Building makefile for ${PROJECT_NAME} v${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")

configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in config.h)
target_include_directories(${PROJECT_EXECUTABLE} PRIVATE ${PROJECT_BINARY_DIR})

# #######################################
# Linking Main against the library
# #######################################
target_link_libraries(${PROJECT_EXECUTABLE} ${PROJECT_LIBRARY})

# #######################################
# Testing
# #######################################
# get googletest from github
find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)

    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

# CHECK ALL THE SUBMODULES
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The submodules was not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# google test is a git submodule for the project, and it is also cmake-based
add_subdirectory(./googletest)

enable_testing()

# Include the gtest library
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# #######################################
# Test files
# #######################################
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

# #######################################
# Unit Tests
# #######################################
add_executable(${PROJECT_TESTS} ${TEST_SRC_FILES})
target_include_directories(${PROJECT_TESTS} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_TESTS} PRIVATE ${COMMON_INCLUDES})
target_include_directories(${PROJECT_TESTS} PRIVATE ${PROJECT_BINARY_DIR})

# Link gtest and app code
target_link_libraries(${PROJECT_TESTS} gtest gtest_main)
target_link_libraries(${PROJECT_TESTS} ${PROJECT_LIBRARY})

# This is so you can do 'make test' to see all your tests run, instead of
# manually running the executable ${PROJECT_TESTS} to see those specific tests.
include(GoogleTest)
gtest_discover_tests(${PROJECT_TESTS})

# #######################################
# Install to /usr/local/bin with `sudo make install`
# #######################################
install(TARGETS ${PROJECT_EXECUTABLE} DESTINATION bin)
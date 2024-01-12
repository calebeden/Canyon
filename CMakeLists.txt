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

# Set the default build type to Release if not provided
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
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
# Compiler Options
# #######################################
target_compile_options(${PROJECT_EXECUTABLE} PRIVATE
    $<$<CONFIG:Debug>: -O0 -g -Werror>
    $<$<CONFIG:Release>: -O3>
)
target_compile_definitions(${PROJECT_LIBRARY} PRIVATE
    $<$<CONFIG:Debug>: DEBUG_TEST_MODE>
    $<$<CONFIG:Release>: RELEASE>
)
target_compile_definitions(${PROJECT_EXECUTABLE} PRIVATE
    $<$<CONFIG:Debug>: DEBUG_TEST_MODE>
    $<$<CONFIG:Release>: RELEASE>
)

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
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

# #######################################
# Test files
# #######################################
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/unit_tests/*.cpp)

# #######################################
# Unit Tests
# #######################################
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_executable(${PROJECT_TESTS} ${TEST_SRC_FILES})
    target_include_directories(${PROJECT_TESTS} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_TESTS} PRIVATE ${COMMON_INCLUDES})
    target_include_directories(${PROJECT_TESTS} PRIVATE ${PROJECT_BINARY_DIR})

    # Link gtest and app code
    target_link_libraries(${PROJECT_TESTS} gtest gtest_main gmock gmock_main)
    target_link_libraries(${PROJECT_TESTS} ${PROJECT_LIBRARY})

    target_compile_options(${PROJECT_TESTS} PRIVATE -O0 -g)
    target_compile_definitions(${PROJECT_TESTS} PRIVATE DEBUG_TEST_MODE)

    # This is so you can do 'make test' to see all your tests run, instead of
    # manually running the executable ${PROJECT_TESTS} to see those specific tests.
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_TESTS})

    # #######################################
    # End to End Tests
    # #######################################
    find_package(Python COMPONENTS Interpreter Development REQUIRED)

    # Using https://github.com/c4deszes/cmake-pytest-module
    include(cmake-pytest-module/Pytest.cmake)

    pytest_discover_tests(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
        EXECUTION_ARGS --rootdir=${CMAKE_CURRENT_SOURCE_DIR}/test
        TEST_PREFIX end_to_end_tests.
    )
endif()

# #######################################
# Install to /usr/local/bin with `sudo make install`
# #######################################
install(TARGETS ${PROJECT_EXECUTABLE} DESTINATION bin)
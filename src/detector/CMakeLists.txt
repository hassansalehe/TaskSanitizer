cmake_minimum_required(VERSION 3.4.3)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

## Set destination directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")

## Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../runtime)

# List of source files for the checker module
add_executable(DFchecker main.cpp checker.cpp validator.cpp)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(DFchecker PRIVATE cxx_range_for cxx_auto_type)

# Add compiler flags. LLVM is (typically) built with no C++ RTTI.
set_target_properties(
     DFchecker PROPERTIES
     COMPILE_FLAGS "-g -O3 -std=c++11 -fno-rtti -fPIC")

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(
    DFchecker
    PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

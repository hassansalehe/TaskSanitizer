#/////////////////////////////////////////////////////////////////
#//  TaskSanitizer: a lightweight non-determinism checking
#//          tool for OpenMP applications
#//
#//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
#//      Copying or using this code by any means whatsoever
#//      without consent of the owner is strictly prohibited.
#//
#//   Contact: hmatar-at-ku-dot-edu-dot-tr
#//
#/////////////////////////////////////////////////////////////////

cmake_minimum_required(VERSION 3.4.3)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

## Set destination directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")

# List source files for libraries and compiler passes.
add_library(TaskSanitizer MODULE TaskSanitizer.cpp)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(TaskSanitizer PRIVATE cxx_range_for cxx_auto_type)

# Add compiler flags. LLVM is (typically) built with no C++ RTTI.
set_target_properties(TaskSanitizer PROPERTIES
     COMPILE_FLAGS "-g -O3 -std=c++11 -fno-rtti -fPIC")

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(
    TaskSanitizer PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

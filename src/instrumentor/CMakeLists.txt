cmake_minimum_required(VERSION 3.4.3)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../detector
                    ${CMAKE_CURRENT_SOURCE_DIR}/../detector/commutativity)
link_directories(${LLVM_LIBRARY_DIRS})

## Set destination directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")

# List source files for libraries.
add_library(Logger STATIC
            eventlogger/Logger.cc
            callbacks/InstrumentationCallbacks.cc
            ../detector/nondeterminism/checker.cc
            ../detector/commutativity/CommutativityChecker.cc)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(Logger PRIVATE cxx_range_for cxx_auto_type)

# Add compiler flags. LLVM is (typically) built with no C++ RTTI.
set_target_properties(Logger PROPERTIES
     COMPILE_FLAGS "-g -O3 -std=c++11 -fno-rtti -fPIC")

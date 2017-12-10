cmake_minimum_required(VERSION 3.4.3)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# List source files for libraries.
add_library(Logger STATIC Logger.cpp)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(Logger PRIVATE cxx_range_for cxx_auto_type)

# Add compiler flags. LLVM is (typically) built with no C++ RTTI.
set_target_properties(Logger PROPERTIES
     COMPILE_FLAGS "-g -O3 -std=c++11 -fno-rtti -fPIC")

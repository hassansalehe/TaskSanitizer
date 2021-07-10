##################################################################
##  TaskSanitizer: a lightweight determinacy race checking
##          tool for OpenMP task applications
##
##    Copyright (c) 2015 - 2021 Hassan Salehe Matar
##      Copying or using this code by any means whatsoever
##      without consent of the owner is strictly prohibited.
##
##   Contact: hassansalehe-at-gmail-dot-com
##
##################################################################

cmake_minimum_required(VERSION 3.4.3)

enable_testing()

# Locate GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../bin/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/detector/commutativity)

add_compile_options(-g -O0 -Wall -fprofile-arcs -ftest-coverage -fpermissive)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
link_libraries(${GTEST_LIBRARIES} pthread gtest_main gcov --coverage)

# Link commonTests with what we want to test and
# the GTest and pthread library
add_executable(commonDefsTests Common_Defs_gtest.cc)
add_executable(commonCritalSigTests Common_CriticalSignatures_gtest.cc)
add_executable(commonMemoryActionsTests Common_MemoryActions_gtest.cc)
add_executable(commonInstructionTests Common_Instruction_gtest.cc)

# Add tests for Ctest
add_test(common_defs_tests, commonDefsTests)
add_test(common_critical_signatures_tests, commonCritalSigTests)
add_test(common_memory_actions_tests, commonMemoryActionsTests)
add_test(common_instruction_tests, commonInstructionTests)

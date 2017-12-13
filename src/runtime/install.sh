#!/bin/bash

#########################################################################
#
# Copyright (c) 2017 - Hassan Salehe Matar
#
#  License: Follows License of LLVM/Clang. Read the licence file LICENSE.md
#
#
# This script builds and installs the EmbedSanitizer race detection runtime
# for 32-bit ARM
#
#########################################################################

# Variables
DEST=.
tSanLib=libflowsan

# Function to Check if a prior command was successful
checkIfActionOK() {
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31m ERROR\033[m"
    exit 1
  fi
}

# Compile and build static library

clang++ -c -L./build -lLogger tsan_interface.cc -o ${tSanLib}.o	\
    -static -std=c++11 -pthread -fpermissive	\
    -fopenmp -L/home/hmatar/Research/FlowSanitizer/bin/lib -Wl,-rpath=/home/hmatar/Research/FlowSanitizer/bin/lib	\
    -I/home/hmatar/Research/FlowSanitizer/bin/include
checkIfActionOK

# Generate etsan as static library
ar rcs ${tSanLib}.a ${tSanLib}.o
checkIfActionOK

echo -e "\033[1;32m etsan runtime library successful installed.\033[m"

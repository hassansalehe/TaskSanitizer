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
taSanLib=libtasksan

# Function to Check if a prior command was successful
checkIfActionOK() {
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31m ERROR\033[m"
    exit 1
  fi
}

# Compile and build static library

clang++ -c MemoryCallbacks.cc -o ${taSanLib}.o	\
    -g -static -std=c++11 -pthread -fpermissive	\
    -fopenmp -I/home/hmatar/Research/TaskSanitizer/bin/include	\
    -I/home/hmatar/Research/TaskSanitizer/src/detector 	\
    -I/home/hmatar/Research/TaskSanitizer/src/runtime
checkIfActionOK

# Generate etsan as static library
ar rcs ${taSanLib}.a ${taSanLib}.o
checkIfActionOK
mv ${taSanLib}.a ../../bin/
echo -e "\033[1;32m TaskSanitizer runtime library successful installed.\033[m"

#!/usr/bin/env bash

# Copyright (c) 2017 - 2018, Hassan Salehe Matar
# All rights reserved.
#
# This file is part of TaskSanitizer. For details, see
# https://github.com/hassansalehe/TaskSanitizer. Please also see the LICENSE file
# for additional BSD notice
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Check if previous command was successful, exit script otherwise.
reportIfSuccessful() {
  if [ $? -eq 0 ]; then
    echo -e "\033[1;32m Done.\033[m"
    return 0
  else
    echo -e "\033[1;31m Fail.\033[m"
    exit 1
  fi
}

fsanHomeDir=`pwd`

# Download the OpenMP runtime
if [ ! -e "${fsanHomeDir}/openmp" ]; then
  git clone https://github.com/llvm-mirror/openmp.git openmp
fi
export OPENMP_INSTALL=${fsanHomeDir}/bin
mkdir -p $OPENMP_INSTALL

cd openmp
mkdir -p build && cd build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL	\
 -D LIBOMP_OMPT_SUPPORT=on -D LIBOMP_OMPT_BLAME=on -D LIBOMP_OMPT_TRACE=on ..

ninja -j8 -l8
ninja install
reportIfSuccessful

# Download Archer tool
cd ${fsanHomeDir}
if [ ! -e "${fsanHomeDir}/archer" ]; then
  git clone https://github.com/PRUNERS/archer.git archer
fi
export ARCHER_INSTALL=${fsanHomeDir}/bin

cd archer
mkdir -p build && cd build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D OMP_PREFIX:PATH=$OPENMP_INSTALL -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} ..

ninja -j8 -l8
ninja install
reportIfSuccessful

# Download the KaSTORs benchmark applications
cd ${fsanHomeDir}
if [ ! -e "${fsanHomeDir}/kastors" ]; then
  git clone https://scm.gforge.inria.fr/anonscm/git/kastors/kastors.git kastors
fi

# Build TaskSanitizer

cd src/runtime
./install.sh
reportIfSuccessful

mkdir -p build
cd -
cd src/runtime/build
rm -rf libLogger.a
CXX=clang++ cmake ..
make
reportIfSuccessful

cd -
mkdir -p build
cd build
rm -rf libTaskSanitizer.so
CXX=clang++ cmake ../src/tsan
make
reportIfSuccessful

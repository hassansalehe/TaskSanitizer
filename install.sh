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

taskSanHomeDir=`pwd`
version=6.0.0

# Checks if necessary packages are installed in the machine
# Installs the packages if user approves.
CheckPrerequisites() {
  which wget > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! wget missing\033[m"
    read -p "Do you want to install wget (root password is needed) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then sudo apt-get install wget; fi
  fi

  which tar > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! tar missing\033[m"
    read -p "Do you want to install tar (root password is needed) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then sudo apt-get install tar; fi
  fi

  # check if cmake is installed
  which cmake > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! cmake missing\033[m"
    read -p "Do you want to install cmake (root password is needed)? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      cd /usr/src
      sudo wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
      sudo tar -xvzf cmake-3.10.3.tar.gz
      cd cmake-3.10.3
      sudo ./bootstrap
      sudo make -j $procNo
      sudo make install
      cd $taskSanHomeDir
    fi
  fi

  # check if Clang compiler is installed
  which clang++ > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! Clang compiler missing\033[m"
    read -p "Do you want to install LLVM/Clang (root password is needed)? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then

      SrcDir=/tmp/LLVM
      mkdir -p $SrcDir
      cd $SrcDir
      cd /usr/src/LLVM
      wget -c http://releases.llvm.org/${version}/llvm-${version}.src.tar.xz
      tar xf llvm-${version}.src.tar.xz --strip-components 1
      rm llvm-${version}.src.tar.xz

      cd $SrcDir
      mkdir -p tools/clang
      cd tools/clang
      wget -c http://releases.llvm.org/${version}/cfe-${version}.src.tar.xz
      tar xf cfe-${version}.src.tar.xz --strip-components 1
      rm cfe-${version}.src.tar.xz

      mkdir -p tools/extra
      cd tools/extra
      wget -c http://releases.llvm.org/${version}/clang-tools-extra-${version}.src.tar.xz
      tar xf clang-tools-extra-${version}.src.tar.xz --strip-components 1
      rm clang-tools-extra-${version}.src.tar.xz

      cd $SrcDir
      mkdir -p projects/compiler-rt
      cd projects/compiler-rt
      wget -c http://releases.llvm.org/${version}/compiler-rt-${version}.src.tar.xz
      tar xf compiler-rt-${version}.src.tar.xz --strip-components 1
      rm compiler-rt-${version}.src.tar.xz

      cd $SrcDir
      mkdir -p projects/libcxx
      cd  projects/libcxx
      wget -c http://releases.llvm.org/${version}/libcxx-${version}.src.tar.xz
      tar xf libcxx-${version}.src.tar.xz --strip-components 1
      rm libcxx-${version}.src.tar.xz

      cd $SrcDir
      mkdir -p projects/libcxxabi
      cd projects/libcxxabi
      wget -c http://releases.llvm.org/${version}/libcxxabi-${version}.src.tar.xz
      tar xf libcxxabi-${version}.src.tar.xz --strip-components 1
      rm libcxxabi-${version}.src.tar.xz

      cd $SrcDir
      mkdir -p projects/libunwind
      cd projects/libunwind
      wget -c http://releases.llvm.org/${version}/libunwind-${version}.src.tar.xz
      tar xf libunwind-${version}.src.tar.xz --strip-components 1
      rm libunwind-${version}.src.tar.xz

      cd $SrcDir
      mkdir -p projects/test-suite
      cd projects/test-suite
      wget -c http://releases.llvm.org/${version}/test-suite-${version}.src.tar.xz
      tar xf test-suite-${version}.src.tar.xz --strip-components 1
      rm test-suite-${version}.src.tar.xz

      echo "Building LLVM"
      mkdir /tmp/llvm-build
      cd /tmp/llvm-build
      sudo cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr $SrcDir
      procNo=`cat /proc/cpuinfo | grep processor | wc -l`
      sudo make -j $procNo
      sudo make install
      # Report if everything is OK so far
      reportIfSuccessful
    fi
  fi

  # check if Ninja build system is installed
  which ninja > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! Ninja build system missing\033[m"
    read -p "Do you want to install Ninja (root password is needed)? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      cd /tmp
      git clone git://github.com/ninja-build/ninja.git && cd ninja
      git checkout release
      ./configure.py --bootstrap
      sudo mv ninja /usr/bin/
      cd $taskSanHomeDir
    fi
  fi

}

# Step 0: Check for prerequisites and install necessary packages
  CheckPrerequisites

# Step 1: Download the OpenMP runtime
if [ ! -e "${taskSanHomeDir}/openmp" ]; then
  git clone https://github.com/llvm-mirror/openmp.git openmp
fi
export OPENMP_INSTALL=${taskSanHomeDir}/bin
mkdir -p $OPENMP_INSTALL

cd openmp
mkdir -p build && cd build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL	\
 -D LIBOMP_OMPT_SUPPORT=on -D LIBOMP_OMPT_BLAME=on -D LIBOMP_OMPT_TRACE=on ..

ninja -j8 -l8
ninja install
reportIfSuccessful

# Step 2: Download Archer tool
cd ${taskSanHomeDir}
if [ ! -e "${taskSanHomeDir}/archer" ]; then
  git clone https://github.com/PRUNERS/archer.git archer
fi
export ARCHER_INSTALL=${taskSanHomeDir}/bin

cd archer
mkdir -p build && cd build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D OMP_PREFIX:PATH=$OPENMP_INSTALL -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} ..

ninja -j8 -l8
ninja install
reportIfSuccessful

# Step 3: Download the KaSTORs benchmark applications
cd ${taskSanHomeDir}
if [ ! -e "${taskSanHomeDir}/kastors" ]; then
  git clone https://scm.gforge.inria.fr/anonscm/git/kastors/kastors.git kastors
fi

# Step 4: Build TaskSanitizer instrumentation module
mkdir -p ${taskSanHomeDir}/build/libLogger
cd ${taskSanHomeDir}/build/libLogger
rm -rf libLogger.a
CXX=clang++ cmake ${taskSanHomeDir}/src/instrumentor
make
reportIfSuccessful

mkdir -p ${taskSanHomeDir}/build/taskSanbuild
cd ${taskSanHomeDir}/build/taskSanbuild
rm -rf libTaskSanitizer.so
CXX=clang++ cmake ${taskSanHomeDir}/src/instrumentor/pass/
make
reportIfSuccessful

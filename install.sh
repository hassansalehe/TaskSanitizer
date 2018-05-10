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

taskSanHomeDir=`pwd`
ThirdPartyDir=${taskSanHomeDir}/thirdparty
mkdir -p ${ThirdPartyDir}

buildsDir=${taskSanHomeDir}/.builds
mkdir -p ${buildsDir}
LLVMversion=5.0.0

# Check if previous command was successful, exit script otherwise.
reportIfSuccessful() {
  if [ $? -eq 0 ]; then
    echo -e "\033[1;32mSUCCESS: $1.\033[m"
    return 0
  else
    echo -e "\033[1;31mFAILURE: $1.\033[m"
    exit 1
  fi
}

CheckAndInstallPython() {
  which python > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mFAILURE: No Python found.\033[m"
      sudo apt-get install -y python python-matplotlib
    return 1
  else
    echo -e "\033[1;32mSUCCESS: Python found.\033[m"
    dpkg -l python-matplotlib | grep "python-matplotlib" > /dev/null
    if [ $? -ne 0 ]; then
      echo -e "\033[1;31mFAILURE: No Python found.\033[m"
      sudo apt-get install -y python-matplotlib
      return 1
    else
      echo -e "\033[1;32mSUCCESS: Python matplotlib found.\033[m"
      return 0
    fi
  fi
  return 0
}

## This function checks version of installed CMake.
## It succeeds if Cmake version is >= 3.8.1
checkCmakeVersion() {
  version=`cmake --version | grep -i -m 1 version | egrep -o "[0-9]+\.[0-9]+\.[0-9]+" `
  first=`echo ${version}  | sed 's/\./ /g' | awk '{print $1}' `
  second=`echo ${version} | sed 's/\./ /g' | awk '{print $2}' `

  if [ "${first}" -gt "3" ]; then
    echo -e "\033[1;32mSUCCESS: CMake (version ${version}) found.\033[m"
    return 0
  elif [ "${first}" -eq "3" ] && [ "${second}" -gt "7" ]; then
    echo -e "\033[1;32mSUCCESS: CMake (version ${version}) found.\033[m"
    return 0
  fi

  echo -e "\033[1;31mFAILURE: No CMake version >= 3.8.0 found.\033[m"
  exit 1
}

BuildInstallCMake() {
  cd ${ThirdPartyDir}
  wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
  tar -xvzf cmake-3.10.3.tar.gz
  cd cmake-3.10.3
  ./bootstrap
  make -j $procNo
  sudo make install
  cd $taskSanHomeDir
}


## This function checks version of installed Ninja-build system.
## It succeeds if Ninja-build version is >= 1.8.0
checkNinjaVersion() {
  version=`ninja --version | egrep -o "[0-9]+\.[0-9]+\.[0-9]+" `
  first=`echo ${version}  | sed 's/\./ /g' | awk '{print $1}' `
  second=`echo ${version} | sed 's/\./ /g' | awk '{print $2}' `

  if [ "${first}" -gt "1" ]; then
    echo -e "\033[1;32mSUCCESS: Ninja build (version ${version}) found.\033[m"
    return 0
  elif [ "${first}" -eq "1" ] && [ "${second}" -gt "7" ]; then
    echo -e "\033[1;32mSUCCESS: Ninja build (version ${version}) found.\033[m"
    return 0
  fi

  echo -e "\033[1;31mFAILURE: No Ninja build version >= 3.8.0 found.\033[m"
  exit 1
}

BuildInstallNinjaBuildSystem() {
  cd ${ThirdPartyDir}
  git clone git://github.com/ninja-build/ninja.git && cd ninja
  git checkout release
  ./configure.py --bootstrap
  sudo mv ninja /usr/bin/
  cd $taskSanHomeDir
}

CheckLLVMClang() {
  which clang++ > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mFAILURE: LLVM/Clang compiler missing\033[m"
    return 1
  else
    find /usr/include/ -name "Instrumentation.h" > /dev/null
    if [ $? -ne 0 ]; then
      echo -e "\033[1;31mFAILURE: LLVM/Clang compiler missing\033[m"
      return 1
    fi
  fi
  echo -e "\033[1;32mSUCCESS: LLVM/Clang compiler found\033[m"
  return 0
}

BuildInstallLLVMClang() {
  llvmDir=${ThirdPartyDir}/llvm
  mkdir -p ${llvmDir} && cd ${llvmDir}
  wget -c http://releases.llvm.org/${LLVMversion}/llvm-${LLVMversion}.src.tar.xz
  tar xf llvm-${LLVMversion}.src.tar.xz --strip-components 1
  rm llvm-${LLVMversion}.src.tar.xz

  cd ${llvmDir}
  mkdir -p tools/clang && cd tools/clang
  wget -c http://releases.llvm.org/${LLVMversion}/cfe-${LLVMversion}.src.tar.xz
  tar xf cfe-${LLVMversion}.src.tar.xz --strip-components 1
  rm cfe-${LLVMversion}.src.tar.xz

  mkdir -p tools/extra && cd tools/extra
  wget -c http://releases.llvm.org/${LLVMversion}/clang-tools-extra-${LLVMversion}.src.tar.xz
  tar xf clang-tools-extra-${LLVMversion}.src.tar.xz --strip-components 1
  rm clang-tools-extra-${LLVMversion}.src.tar.xz

  cd ${llvmDir}
  mkdir -p projects/compiler-rt && cd projects/compiler-rt
  wget -c http://releases.llvm.org/${LLVMversion}/compiler-rt-${LLVMversion}.src.tar.xz
  tar xf compiler-rt-${LLVMversion}.src.tar.xz --strip-components 1
  rm compiler-rt-${LLVMversion}.src.tar.xz

  cd ${llvmDir}
  mkdir -p projects/libcxx && cd projects/libcxx
  wget -c http://releases.llvm.org/${LLVMversion}/libcxx-${LLVMversion}.src.tar.xz
  tar xf libcxx-${LLVMversion}.src.tar.xz --strip-components 1
  rm libcxx-${LLVMversion}.src.tar.xz

  cd ${llvmDir}
  mkdir -p projects/libcxxabi && cd projects/libcxxabi
  wget -c http://releases.llvm.org/${LLVMversion}/libcxxabi-${LLVMversion}.src.tar.xz
  tar xf libcxxabi-${LLVMversion}.src.tar.xz --strip-components 1
  rm libcxxabi-${LLVMversion}.src.tar.xz

  cd ${llvmDir}
  mkdir -p projects/libunwind && cd projects/libunwind
  wget -c http://releases.llvm.org/${LLVMversion}/libunwind-${LLVMversion}.src.tar.xz
  tar xf libunwind-${LLVMversion}.src.tar.xz --strip-components 1
  rm libunwind-${LLVMversion}.src.tar.xz

  echo "Building LLVM"
  mkdir -p ${llvmDir}/build && cd ${llvmDir}/build
  sudo cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ${llvmDir}
  procNo=`cat /proc/cpuinfo | grep processor | wc -l`
  sudo make -j ${procNo}
  sudo make install
  # Report if everything is OK so far
  reportIfSuccessful "Building LLVM/Clang"
}

# Checks if necessary packages are installed in the machine
# Installs the packages if user approves.
CheckPrerequisites() {
  which wget > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! wget missing\033[m"
    read -p "Do you want to install wget (root password is needed)? [N/y] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then sudo apt-get install -y wget; fi
  fi

  which tar > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! tar missing\033[m"
    read -p "Do you want to install tar (root password is needed)? [N/y]" -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then sudo apt-get install -y tar; fi
  fi

  # check if cmake is installed
  which cmake > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! cmake missing\033[m"
    read -p "Do you want to install cmake (root password is needed)? [N/y] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      sudo apt-get install -y cmake
      which cmake > /dev/null
      if [ $? -eq 0 ]; then
        checkCmakeVersion
        if [ $? -ne 0 ]; then
          BuildInstallCMake
        fi
      fi
    fi
  fi
  checkCmakeVersion

  # check if Clang compiler is installed
  CheckLLVMClang
  if [ $? -ne 0 ]; then
    read -p "Do you want to install LLVM/Clang (root password is needed)? [N/y] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      BuildInstallLLVMClang
    fi
  fi

  # check if Ninja build system is installed
  which ninja > /dev/null
  if [ $? -ne 0 ]; then
    echo -e "\033[1;31mERROR! Ninja build system missing\033[m"
    read -p "Do you want to install Ninja (root password is needed)? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      BuildInstallNinjaBuildSystem
    fi
  fi
  checkNinjaVersion

  CheckAndInstallPython
}

# Step 0: Check for prerequisites and install necessary packages
  CheckPrerequisites

# Step 1: Download the OpenMP runtime
if [ ! -e "${ThirdPartyDir}/openmp" ]; then
  cd ${ThirdPartyDir}
  git clone https://github.com/llvm-mirror/openmp.git openmp
fi
export OPENMP_INSTALL=${taskSanHomeDir}/bin
mkdir -p $OPENMP_INSTALL

openmpsrc=${ThirdPartyDir}/openmp
mkdir -p ${buildsDir}/openmp-build && cd ${buildsDir}/openmp-build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL	\
 -D LIBOMP_OMPT_SUPPORT=on -D LIBOMP_OMPT_BLAME=on -D LIBOMP_OMPT_TRACE=on ${openmpsrc}

ninja -j8 -l8
ninja install
reportIfSuccessful "Compiling OpenMP runtime"

# Step 2: Download Archer tool
if [ ! -e "${ThirdPartyDir}/archer" ]; then
  cd ${ThirdPartyDir}
  git clone https://github.com/PRUNERS/archer.git archer
fi
export ARCHER_INSTALL=${taskSanHomeDir}/bin

archersrc=${ThirdPartyDir}/archer
mkdir -p ${buildsDir}/archer-build && cd ${buildsDir}/archer-build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D OMP_PREFIX:PATH=$OPENMP_INSTALL -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} ${archersrc}

ninja -j8 -l8
ninja install
reportIfSuccessful "Compiling Archer race detection tool"

# Step 4: Build TaskSanitizer instrumentation module
mkdir -p ${buildsDir}/libLogger-build && cd ${buildsDir}/libLogger-build
rm -rf libLogger.a
CXX=clang++ cmake -G Ninja ${taskSanHomeDir}/src/instrumentor
ninja
reportIfSuccessful "Compiling TaskSanitizer runtime"

mkdir -p ${buildsDir}/tasksan-build && cd ${buildsDir}/tasksan-build
rm -rf libTaskSanitizer.so
CXX=clang++ cmake -G Ninja ${taskSanHomeDir}/src/instrumentor/pass/
ninja
reportIfSuccessful "Compiling TaskSanitizer instrumentation"

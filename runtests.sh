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

reset

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

cd ..
reset

TESTS=($(grep -l main src/tests/benchmarks/*.cc))
for test in "${TESTS[@]}"; do
  rm -rf Trace* HBlog*
  rm -rf a.out
  echo "RUNNING $test"
  tasksan $test -std=c++11
  ./a.out 12
done


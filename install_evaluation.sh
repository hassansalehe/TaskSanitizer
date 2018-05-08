taskSanHomeDir=`pwd`
ThirdPartyDir=${taskSanHomeDir}/thirdparty
mkdir -p ${ThirdPartyDir}

buildsDir=${taskSanHomeDir}/.builds
mkdir -p ${buildsDir}
version=5.0

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


sudo apt update
sudo apt upgrade

sudo apt-get install -y clang-${version}
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${version} 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${version} 100
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-${version} 100

sudo apt-get install -y cmake
sudo apt-get install -y ninja-build
sudo apt-get install -y python-minimal

export OPENMP_INSTALL=${taskSanHomeDir}/bin
mkdir -p $OPENMP_INSTALL
openmpsrc=${ThirdPartyDir}/openmp
mkdir -p ${buildsDir}/openmp-build && cd ${buildsDir}/openmp-build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++  \
 -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL       \
 -D LIBOMP_OMPT_SUPPORT=on -D LIBOMP_OMPT_BLAME=on -D LIBOMP_OMPT_TRACE=on ${openmpsrc}

ninja -j8 -l8
ninja install
reportIfSuccessful

export ARCHER_INSTALL=${taskSanHomeDir}/bin
archersrc=${ThirdPartyDir}/archer
mkdir -p ${buildsDir}/archer-build && cd ${buildsDir}/archer-build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++  \
 -D OMP_PREFIX:PATH=$OPENMP_INSTALL -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} ${archersrc}

ninja -j8 -l8
ninja install
reportIfSuccessful

# Step 4: Build TaskSanitizer instrumentation module
mkdir -p ${buildsDir}/libLogger-build && cd ${buildsDir}/libLogger-build
rm -rf libLogger.a
CXX=clang++ cmake -G Ninja ${taskSanHomeDir}/src/instrumentor
ninja
reportIfSuccessful

mkdir -p ${buildsDir}/tasksan-build && cd ${buildsDir}/tasksan-build
rm -rf libTaskSanitizer.so
CXX=clang++ cmake -G Ninja ${taskSanHomeDir}/src/instrumentor/pass/
ninja
reportIfSuccessful

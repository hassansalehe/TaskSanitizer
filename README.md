### TaskSanitizer: Runtime Determinacy Race Detection Tool for OpenMP Tasks
[![codecov](https://codecov.io/gh/hassansalehe/TaskSanitizer/branch/master/graph/badge.svg?token=5YCIUJZ6U9)](https://codecov.io/gh/hassansalehe/TaskSanitizer)
[![Build Status](https://github.com/hassansalehe/TaskSanitizer/actions/workflows/C_Integration.yml/badge.svg)](https://github.com/hassansalehe/TaskSanitizer/actions/workflows/C_Integration.yml)
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fhassansalehe%2FTaskSanitizer.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fhassansalehe%2FTaskSanitizer?ref=badge_shield)

#### Overview
TaskSanitizer implements a method to detect determinacy races in OpenMP tasks.
It relies on open-source tools and is mostly written in C++. It launches
through a custom Bash script called *__tasksan__* which contains all necessary
command-line arguments to compile and instrument a C/C++ OpenMP program.
Moreover, it depends on LLVM/Clang compiler infrastructure and contains a
compiler pass which instruments the program undertest to identify all necessary
features, such as memory accesses, and injects race detection runtime into the
produced binary of the program. Race detection warnings are displayed on the
standard output while the instrumented binary executes. TaskSanitizer also
relies on LLVM’s OpenMP runtime (https://github.com/llvm-mirror/openmp) which
contains runtime interface for performance and correctness tools (OMPT). OMPT
signals events for various OpenMP events -- such as creation of a task, task
scheduling, etc. -- and TaskSanitizer implements necessary callbacks for these
events for tacking and categorizing program events of each task in the program.

TaskSanitizer has been developed entirely on Linux Ubuntu on versions 16.04.4
LTS and 18.04 LTS. However, it does not use any unique features of Ubuntu and
thus it can be tested on any Linux/Unix system which has access to a terminal
and Bash program. It also requires LLVM/Clang with version >= 5.0.0, CMake
version >= 3.8, Ninja build system version >= 1.8, and Python version >= 2.7.12.
We recommend Ubuntu 18.04 LTS since has it these tools in its standard repository
and therefore the process for installing them is simplified. In Section 2 we
will provide step-by-step instructions on how to install these tools on Ubuntu
18.04 LTS.

Finally, our tool source code size is less than 5 MB and it occupies
80 MB after installation. Nevertheless, one-time compilation builds of release
version of LLVM/Clang requires at least 10 GB of disk space and its installation
occupies 1.5GB of harddisk.

#### Building TaskSanitizer

This section explains how to install necessary tools for building and running
TaskSanitizer in a Linux system. Most commands used here for installing these
tools are Debian-based but equivalent commands can easily be inferred.

###### 1. Updating System
It is recommended to have an updated system. For Debian-based systems (Ubuntu
included), the following commands suffice to update.

```bash
sudo apt-get update && sudo apt-get upgrade
```

###### 2. Basic Linux tools
Installation script of TaskSanitizer as well as its launcher script use grep,
sed, perl, tar, g++ basic program utilities and executes on Linux Bash. These
tools are available by default in recent Linux systems. The commands below are
used to download and install Linux Bash if it does not exist in the system.

```bash
sudo apt-get install build-essential
wget http://ftp.gnu.org/gnu/bash/bash-4.4.tar.gz
tar xf bash-4.4.tar.gz
cd bash-4.4
./configure
make
sudo make install
```

Next, the command below can be used to install the remaining tools in case they
are not installed.

```bash
sudo apt-get install sed perl tar g++
```
###### 3. Installing CMake
CMake with at least version 3.8 is used for compiling modules of TaskSanitizer
through our automated script install.sh The command below is used for installing
CMake from a Debian repository. It takes around 50 seconds to install CMake
version 3.10.2 which is available by default in Ubuntu 18.04 LTS repository.

```bash
sudo apt-get install -y cmake
```

Alternatively, CMake can be installed from source code using the following list
of commands for the case of CMake 3.10.3

```bash
wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
tar -xvzf cmake-3.10.3.tar.gz
cd cmake-3.10.3
./bootstrap
make -j 4
sudo make install
```

###### 4. Installing Ninja Build System
TaskSanitizer uses Ninja build system to facilitate compilation of its C++ code.
It requires Ninja build with version 1.8.2 or more. Installation of this system
from Debian repository requires 15 seconds with the command below.

```bash
sudo apt-get install -y ninja-build
```

###### 5. Installing LLVM/Clang
LLVM/Clang is the most important tool for TaskSanitizer. It is highly recommended
to build and install LLVM/Clang from source code so that TaskSanitizer uses its
developer features. Additionally, TaskSanitizer has been tested with LLVM/Clang
versions 5.0.0 and 6.0.0. The script install.sh in the main directory of
TaskSanitizer contains necessary commands to download, build, install LLVM/Clang
into a Linux system. The necessary Bash commands are also given below. It is
important to note that this one-time process takes more than 30 minutes and it
depends on system computing capacity. For evaluation purposes of TaskSanitizer,
we suggest downloading and using the virtualization instance provided on Section
2.1 as it already contains LLVM/Clang. Alternatively, the commands below can be
used to build and install LLVM/Clang.

```bash
llvmDir=${PWD}/llvm
mkdir -p ${llvmDir} && cd ${llvmDir}
wget -c http://releases.llvm.org/5.0.0/llvm-5.0.0.src.tar.xz
tar xf llvm-5.0.0.src.tar.xz --strip-components 1 && rm llvm-5.0.0.src.tar.xz

cd ${llvmDir} && mkdir -p tools/clang && cd tools/clang
wget -c http://releases.llvm.org/5.0.0/cfe-5.0.0.src.tar.xz
tar xf cfe-5.0.0.src.tar.xz --strip-components 1 && rm cfe-5.0.0.src.tar.xz
mkdir -p tools/extra && cd tools/extra
wget -c http://releases.llvm.org/5.0.0/clang-tools-extra-5.0.0.src.tar.xz
tar xf clang-tools-extra-5.0.0.src.tar.xz --strip-components 1
rm clang-tools-extra-5.0.0.src.tar.xz

cd ${llvmDir} && mkdir -p projects/compiler-rt && cd projects/compiler-rt
wget -c http://releases.llvm.org/5.0.0/compiler-rt-5.0.0.src.tar.xz
tar xf compiler-rt-5.0.0.src.tar.xz --strip-components 1 && rm compiler-rt-5.0.0.src.tar.xz

cd ${llvmDir}
mkdir -p projects/libcxx && cd projects/libcxx
wget -c http://releases.llvm.org/5.0.0/libcxx-5.0.0.src.tar.xz
tar xf libcxx-5.0.0.src.tar.xz --strip-components 1 && rm libcxx-5.0.0.src.tar.xz

cd ${llvmDir} && mkdir -p projects/libcxxabi && cd projects/libcxxabi
wget -c http://releases.llvm.org/5.0.0/libcxxabi-5.0.0.src.tar.xz
tar xf libcxxabi-5.0.0.src.tar.xz --strip-components 1 && rm libcxxabi-5.0.0.src.tar.xz

cd ${llvmDir} && mkdir -p projects/libunwind && cd projects/libunwind
wget -c http://releases.llvm.org/5.0.0/libunwind-5.0.0.src.tar.xz
tar xf libunwind-5.0.0.src.tar.xz --strip-components 1 && rm libunwind-5.0.0.src.tar.xz

mkdir -p ${llvmDir}/build && cd ${llvmDir}/build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ${llvmDir}
procNo=`cat /proc/cpuinfo | grep processor | wc -l`
ninja -j ${procNo}
sudo ninja install
```

###### 6. Building TaskSanitizer
*__install.sh__* is a Linux Bash script which compiles all necessary components
of TaskSanitizer. Moreover, it contains necessary script to check if all tools
are installed in the system. Then it builds OpenMP runtime, Archer race detection
tool which used for evaluation comparison as presented on Table 2 in the paper.
Finally, it compiles instrumentation and runtime modules of TaskSanitizer.
To run install.sh, one can change terminal position to the main directory of
TaskSanitizer and issue the command below.

```bash
./install.sh
```
The produced binaries are stored in bin folder under the main folder of
TaskSanitizer. Messages output from install.sh tells status of each compilation.

#### Using TaskSanitizer to instrument OpenMP C/C++ programs
This section depends on successful building TaskSanitizer. To facilitate
evaluation, we developed a simple Python script *__evaluation.py__* under the
main directory of the project to facilitate evaluation of different aspects of
our method. This section describes how to detect determinacy races on any C/C++.

###### Detecting Determinacy Races in Any C/C++ Program
Detecting determinacy races in a C/C++ program using TaskSanitizer involves stages.
The first stage is instrumentation of the program through TaskSanitizer. This is
achieved with the following command which assumes the terminal is on main
directory of TaskSanitizer.

```bash
./tasksan <your_c_cpp_program_file_name> <custom_compiler flags>
```

For example, `./src/banchmarks/RacyBackgroundExample.cc` can be compiled as below.

```bash
./tasksan ./src/banchmarks/RacyBackgroundExample.cc -O ./RacyBackgroundExample.exe
```

The second stage is execution of the produced binary after instrumentation.
The races are detected while the program executes and are reported to standard
output when it terminates. The screenshot below shows how the produced binary
for `./src/banchmarks/RacyBackgroundExample.cc` is executed and a sample output with
detected race which shows there is a race between line numbers 29 and 33 as a
result of Write-Write conflicts between two concurrent tasks.

```bash
./RacyBackgroundExample.exe
No. of critical sections in IIR: 2
i=1
============================================================
Summary
Total number of tasks: 6
Determinacy race checking report
The following 1 task pairs have conflicts:
29 (29) <--> 33 (33) on 1 memory addresses
0x140728574542584 lines: .omp_outlined..1: 29, .omp_outlined..2: 33 task ids: (1[W] 3[W])
============================================================
```

#### Copyright notice
(c) 2015 - 2021 Hassan Salehe Matar  
All rights reserved.   
Please read license file LICENSE on usage license.

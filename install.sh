hm=`pwd`

git clone https://github.com/llvm-mirror/openmp.git openmp

export OPENMP_INSTALL=$hm/bin
cd openmp/runtime
mkdir build && cd build

cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL	\
 -D LIBOMP_OMPT_SUPPORT=on -D LIBOMP_OMPT_BLAME=on -D LIBOMP_OMPT_TRACE=on ..

ninja -j8 -l8
ninja install

git clone https://github.com/PRUNERS/archer.git archer
export ARCHER_INSTALL=$hm/bin

cd archer

mkdir build && cd build

cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++	\
 -D OMP_PREFIX:PATH=$OPENMP_INSTALL -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} ..

ninja -j8 -l8
ninja install
cd ../..

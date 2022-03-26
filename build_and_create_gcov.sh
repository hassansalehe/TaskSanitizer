BUILD_DIR=`pwd`/build2
rm -rf temp
rm -rf *gcno *gcda *gcov
mkdir -p $BUILD_DIR
cd $BUILD_DIR

rm -rf *
cmake ../test
make -j 2 
ctest -V -C
cd ..
find $BUILD_DIR -iname '*.gcno' -exec cp {} ./ \;
find $BUILD_DIR -iname '*.gcda' -exec cp {} ./ \;
gcov -o . ./test/race_test.cpp --object-directory ./
lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info
lcov --list coverage.info
genhtml coverage.info -o temp
rm -rf *gcno *gcda *gcov

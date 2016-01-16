#!/bin/bash

cd $(dirname "$0")/..

rm -Rf build.coverage

mkdir build.coverage

cd build.coverage || exit 1

cmake -DCMAKE_BUILD_TYPE=Coverage .. || exit 1

make || exit 1
make test || exit 1

lcov --rc lcov_branch_coverage=1 --base-directory . --directory . -c -o abb.info || exit 1
genhtml --branch-coverage --demangle-cpp -o html abb.info || exit 1

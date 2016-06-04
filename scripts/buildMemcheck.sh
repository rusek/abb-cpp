#!/bin/bash

cd $(dirname "$0")/..

rm -Rf build.memcheck

mkdir build.memcheck

cd build.memcheck || exit 1

cmake -DCMAKE_BUILD_TYPE=Memcheck .. || exit 1

make || exit 1
make test || exit 1

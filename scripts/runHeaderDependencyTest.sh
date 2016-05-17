#!/bin/bash

cd $(dirname "$0")/..

shopt -s globstar

for FILE_PATH in include/**/*.h
do
    echo "TEST $FILE_PATH"
    g++ -c -o a.o -std=c++11 -Iinclude "$FILE_PATH" || exit 1
done

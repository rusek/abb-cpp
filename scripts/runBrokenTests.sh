#!/bin/bash

cd $(dirname "$0")/..

for FILE_PATH in tests/broken/*.cpp
do
    echo "TEST $FILE_PATH"
    g++ -c -o a.o -std=c++11 -Iinclude "$FILE_PATH" 2>/dev/null
    RET=$?
    if [ $RET -eq 0 ]
    then
        echo "FAILED"
        exit 1
    fi
done

#!/bin/bash

cd $(dirname "$0")/../..

function compile {
    g++ -c -o a.o -std=c++11 -Iinclude "$@"
}

for FILE_PATH in tests/broken/*.cpp
do
    echo "TEST $FILE_PATH"

    compile "$FILE_PATH" 2>/dev/null
    RET=$?
    if [ $RET -eq 0 ]
    then
        echo "FAILED"
        exit 1
    fi

    compile -DAPPLY_FIX "$FILE_PATH"
    RET=$?
    if [ $RET -ne 0 ]
    then
        echo "FAILED"
        exit 1
    fi
done

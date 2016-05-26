#!/bin/bash

cd $(dirname "$0")/..

mkdir -p build || exit 1
(cd build && cmake .. && make) || exit 1
for FILE_PATH in build/test_*
do
    echo "=== $FILE_PATH"
    "$FILE_PATH" || exit 1
done

for FILE_PATH in tests/scripts/test_*
do
    echo "=== $FILE_PATH"
    "$FILE_PATH" || exit 1
done

#!/bin/bash

cd $(dirname "$0")/..

shopt -s globstar

function getExpectedHeaderGuard {
    python -c "
import re, sys
arg = sys.argv[1]
prefix, suffix = 'include/', '.h'
assert arg.startswith(prefix) and arg.endswith(suffix)
arg = arg[len(prefix): -len(suffix)]
print re.sub('([A-Z])', '_\\\1', arg).replace('/', '_').upper() + '_H'
    " "$1"
}

for FILE_PATH in include/**/*.h
do
    echo "TEST $FILE_PATH"
    EXPECTED_GUARD=`getExpectedHeaderGuard "$FILE_PATH"`
    GUARD_COUNT=`grep "$EXPECTED_GUARD" "$FILE_PATH" | wc -l`
    if [ "$GUARD_COUNT" -ne 3 ]
    then
        echo "Invalid guard, expecting $EXPECTED_GUARD"
        exit 1
    fi
done

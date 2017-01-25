#!/bin/sh
touch CurieBLE.h
rm -rf mourino mourino.dSYM
gcc -o mourino -I. -DEXECUTE_FROM_COMMAND_LINE mourino.cpp -O0 -g
./mourino
if [ $? -eq 0 ]; then
    echo "PASSED"
    rm -rf mourino mourino.dSYM
else
    echo "FAILED"
fi
rm CurieBLE.h

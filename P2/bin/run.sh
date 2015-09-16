#!/bin/bash
echo
echo "Hello, sangyy"

echo
echo "Now compiling"
cd ..
make
cd ./bin

echo
echo "1. Kaleidoscope"
echo "------------------------"

echo 
echo "Now kaleido is running, type in Ctrl+D to go to part 2"
./kaleido

echo
echo "2. Using flex to scan C1 source code"
echo "---------------------------------"
echo

echo "Here are some test file name: ../test/test1.c ../test/text2.c ../test/test3.c"
./c1c

#!/bin/bash

echo "Hello, sangyy"

cd ../src
make > /dev/null
cd ../bin

read -n1 -rsp "Press enter to run test1"
echo
../src/test1

read -n1 -rsp "Press enter to run test2"
echo
../src/test2

read -n1 -rsp "Press enter to run test3"
echo
../src/test3

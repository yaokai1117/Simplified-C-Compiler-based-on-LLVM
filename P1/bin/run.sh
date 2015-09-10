#!/bin/bash

echo "Hello, sangyy"

cd ..
make > /dev/null
cd bin

read -n1 -rsp "Press enter to run test1"
echo
../test1

read -n1 -rsp "Press enter to run test2"
echo
../test2

read -n1 -rsp "Press enter to run test3"
echo
../test3

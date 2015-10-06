#!/bin/bash
cd bin
echo
echo "Hello, sangyy"

echo
echo "Now compiling"
cd ..
make

echo

bin/toy

dot -Tpng toy.dot -o watch.png


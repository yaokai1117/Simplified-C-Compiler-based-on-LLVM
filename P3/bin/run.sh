#!/bin/bash
cd bin
echo
echo "Hello, sangyy"

cd ..
echo
echo "Now compiling"
make

echo

bin/toy

dot -Tpng toy.dot -o watch.png


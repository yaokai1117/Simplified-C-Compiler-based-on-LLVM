#!/bin/bash
cd bin
echo
echo "Hello"

cd ..
echo
echo "Now compiling"
make


while true
do

echo
echo

echo "Please input a number(1~4) to run a test, Ctrl-d to exit:"
echo " 		1 for test1.c -- (genarate DOT) a test with no error or warning"
echo " 		2 for test2.c -- (genarate DOT) a bubble sort program"
echo " 		3 for warn.c  -- (genarate DOT) test1 with a warning"
echo " 		4 for error.c -- (Don't genarate DOT) test1 with 4 errors and a warning"

read choice

case $choice in
	1)
		bin/compiler test/test1.c -d asgn.dot
		./trans.sh
		;;
	2)
		bin/compiler test/test2.c -d asgn.dot
		./trans.sh
		;;
	3)
		bin/compiler test/warn.c -d asgn.dot
		./trans.sh
		;;
	4)
		bin/compiler test/error.c -d asgn.dot
		;;
	*)
		echo $choice: unknown option
		exit 1
		;;
esac


done


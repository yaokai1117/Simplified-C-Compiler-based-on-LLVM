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

echo "Please input a number(1~6) to run a test, Ctrl-d to exit:"
echo " 		1 for test1.c -- basical test to print semantic rules(part1)"
echo " 		2 for test2.c -- basical test to print semantic rules(part2)"
echo " 		3 for test3.c -- a bubble sort program"
echo " 		4 for error1.c -- different kinds of errors"
echo " 		5 for error2.c -- when errors come tegether"
echo " 		6 for error3.c -- bubble sort with some errors"

read choice

case $choice in
	1)
		bin/compiler test/test1.c
		;;
	2)
		bin/compiler test/test2.c
		;;
	3)
		bin/compiler test/test3.c
		;;
	4)
		bin/compiler test/error1.c
		;;
	5)
		bin/compiler test/error2.c
		;;
	6)
		bin/compiler test/error3.c
		;;
	*)
		echo $choice: unknown option
		exit 1
		;;
esac


done


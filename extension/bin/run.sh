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
echo " 		1 for test1.c -- test if-else and while"
echo " 		2 for test2.c -- test array"
echo " 		3 for test3.c -- test function (function can have arguments)"
echo " 		4 for test4.c -- test &&, ||, ! operators"
echo " 		5 for quicksort.c -- a quick sort program"
echo " 		6 for error.c -- test error handling"

read choice

case $choice in
	1)
		bin/compiler test/test1.c 
		llc -filetype=obj test1.ll -o bin/test1.o
		clang bin/test1.o bin/libexternfunc.so -o test1
		./test1
		;;
	2)
		bin/compiler test/test2.c 
		llc -filetype=obj test2.ll -o bin/test2.o
		clang bin/test2.o bin/libexternfunc.so -o test2
		./test2
		;;
	3)
		bin/compiler test/test3.c 
		llc -filetype=obj test3.ll -o bin/test3.o
		clang bin/test3.o bin/libexternfunc.so -o test3
		./test3
		;;
	4)
		bin/compiler test/test4.c 
		llc -filetype=obj test4.ll -o bin/test4.o
		clang bin/test4.o bin/libexternfunc.so -o test4
		./test4
		;;
	5)
		bin/compiler test/quicksort.c 
		llc -filetype=obj quicksort.ll -o bin/quicksort.o
		clang bin/quicksort.o bin/libexternfunc.so -o quicksort
		./quicksort
		;;
	6)
		bin/compiler test/error.c 
		;;

	*)
		echo $choice: unknown option
		exit 1
		;;
esac


done


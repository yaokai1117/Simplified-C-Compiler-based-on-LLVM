#!/bin/sh

# Generate IR
bin/compiler fuck.c 
# Generate Obj
llc -filetype=obj fuck.ll -o fuck.o

# Link
clang fuck.o ./libdemo.so -o fuck

# run
./fuck

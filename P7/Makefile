CC=clang++
LEX=lex
YACC=bison
CFLAGS= -g -I include 
YFLAGS=
LFLAGS=
LLVM_LINK_FLAG=`llvm-config --ldflags --system-libs --libs core mcjit native`

LLVM_CXX_FLAG=`llvm-config --cxxflags|sed 's/-fno-rtti//'` 

all: bin/compiler

bin/compiler: bin/lexer.o bin/parser.o bin/main.o bin/util.o bin/global.o bin/msgfactory.o bin/dumpdot.o bin/node.o bin/codegen.o
	@mkdir -p bin
	$(CC) -o $@ $^ $(LLVM_LINK_FLAG) 


bin/main.o: src/main.cpp include/tok.h include/util.h include/global.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/parser.o: src/parser.cpp include/util.h include/global.h include/msgfactory.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/lexer.o: src/lexer.cpp include/tok.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/util.o: src/util.cpp include/util.h include/global.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/global.o: src/global.cpp include/global.h 
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/node.o: src/node.cpp include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/codegen.o: src/codegen.cpp include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/dumpdot.o: src/dumpdot.cpp include/dumpdot.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/msgfactory.o: src/msgfactory.cpp include/msgfactory.h include/global.h include/util.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

src/lexer.cpp: config/lexer.l config/parser.y
	$(LEX) $(LFLAGS) -o $@ $<

src/parser.cpp include/tok.h: config/parser.y
	$(YACC) $(YFLAGS) -v --defines=include/tok.h -o src/parser.cpp $<



.PHONY: clean
clean:
	rm -f bin/*.o  bin/compiler
	rm -f src/lexer.cpp src/parser.cpp src/parser.output include/tok.h
	rm -f *.png *.dot


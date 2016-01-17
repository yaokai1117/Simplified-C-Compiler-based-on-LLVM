CC=clang++
LEX=lex
YACC=bison
CFLAGS= -g -I include 
YFLAGS=
LFLAGS=
LLVM_LINK_FLAG=`llvm-config --ldflags --system-libs --libs core mcjit native`

LLVM_CXX_FLAG=`llvm-config --cxxflags|sed 's/-fno-rtti//'` 

all: bin/compiler bin/libexternfunc.so

bin/compiler: bin/lexer.o bin/parser.o bin/main.o bin/util.o bin/global.o bin/msgfactory.o bin/dumpdot.o bin/node.o bin/dumpdot_visitor.o bin/codegen_visitor.o bin/check_visitor.o
	@mkdir -p bin
	$(CC) -o $@ $^ $(LLVM_LINK_FLAG) 


bin/main.o: src/main.cpp include/tok.h include/util.h include/global.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/parser.o: src/parser.cpp include/util.h include/global.h include/msgfactory.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

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
	$(CC) $(CFLAGS) -c -o $@ $<

bin/codegen_visitor.o: src/codegen_visitor.cpp include/codegen_visitor.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LLVM_CXX_FLAG) -c -o $@ $<

bin/dumpdot.o: src/dumpdot.cpp include/dumpdot.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/dumpdot_visitor.o: src/dumpdot_visitor.cpp include/dumpdot_visitor.h include/node.h include/dumpdot.h include/visitor.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/check_visitor.o: src/check_visitor.cpp include/check_visitor.h include/node.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/msgfactory.o: src/msgfactory.cpp include/msgfactory.h include/global.h include/util.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

src/lexer.cpp: config/lexer.l config/parser.y include/node.h
	$(LEX) $(LFLAGS) -o $@ $<

src/parser.cpp include/tok.h: config/parser.y include/node.h
	$(YACC) $(YFLAGS) -v --defines=include/tok.h -o src/parser.cpp $<

bin/libexternfunc.so: src/libexternfunc.c
	clang -shared -fPIC -o bin/libexternfunc.so src/libexternfunc.c



.PHONY: clean
clean:
	rm -f bin/*.o bin/*.so  bin/compiler
	rm -f src/lexer.cpp src/parser.cpp src/parser.output include/tok.h
	rm -f *.png *.dot
	rm *.ll 
	rm test1 test2 test3 sort 
	


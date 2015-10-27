CC=clang++
LEX=lex
YACC=bison
CFLAGS=-I include
YFLAGS=
LFLAGS=

all: bin/compiler

bin/compiler: bin/lexer.o bin/parser.o bin/main.o
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

bin/main.o: src/main.cpp include/tok.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/parser.o: src/parser.cpp include/util.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin/lexer.o: src/lexer.cpp include/tok.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<

src/lexer.cpp: config/lexer.l
	$(LEX) $(LFLAGS) -o $@ $<

src/parser.cpp include/tok.h: config/parser.y
	$(YACC) $(YFLAGS) --defines=include/tok.h -o src/parser.cpp $<



.PHONY: clean
clean:
	rm -f bin/*.o src/lexer.cpp src/parser.cpp include/tok.h
	rm -f *.png *.dot

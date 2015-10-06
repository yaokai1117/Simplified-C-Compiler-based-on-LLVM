CC=clang++
LLVMCONF=llvm-config
CFLAGS=-I include

.PHONY:all
all:bin/toy

bin/toy: bin/toy.o bin/dumpdot.o
	$(CC) -o $@ $^ `$(LLVMCONF) --cxxflags --ldflags --system-libs --libs core mcjit native`

bin/toy.o: src/toy.cpp include/ast.h include/dumpdot.h
	@mkdir -p bin
	$(CC) -c -o $@ $< $(CFLAGS) `$(LLVMCONF) --cxxflags|sed 's/-fno-rtti//'`

bin/dumpdot.o: src/dumpdot.cpp include/dumpdot.h
	@mkdir -p bin
	$(CC) -c -o $@ $< $(CFLAGS) `$(LLVMCONF) --cxxflags`

.PHONY:clean
clean:
	-rm -f bin/toy bin/*.o
	-rm -f watch.png toy.dot

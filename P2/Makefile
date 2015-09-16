CC = clang++
LEX = flex
BIN = ./bin
SRC = ./src
INC = ./src
CFLAGS = -I $(INC)
LFLAGS = 


objects = $(BIN)/main.o $(BIN)/lexer.o $(BIN)/tok.o

all : $(BIN)/c1c $(BIN)/kaleido

$(BIN)/c1c : $(objects)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(objects) : $(SRC)/main.cpp $(SRC)/lexer.cpp $(SRC)/tok.cpp
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -c -o $(BIN)/main.o $(SRC)/main.cpp
	$(CC) $(CFLAGS) -c -o $(BIN)/lexer.o $(SRC)/lexer.cpp
	$(CC) $(CFLAGS) -c -o $(BIN)/tok.o $(SRC)/tok.cpp

$(SRC)/lexer.cpp : $(SRC)/lexer.lex
	$(LEX) $(LFLAGS) -o $(SRC)/lexer.cpp $(SRC)/lexer.lex

$(BIN)/kaleido : $(SRC)/kaleidoscope.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean : 
	rm -f $(objects) $(BIN)/c1c $(BIN)/kaleido
	rm -f $(SRC)/lexer.cpp
	


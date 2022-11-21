CC=cc -std=c99 -O2 -Wall -Wextra `pkg-config --cflags raylib`
LIB=`pkg-config --libs raylib` -llua
OBJ=raylua.o rayimp.o
BIN=raylua

default: $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LIB)

clean:
	rm -f $(BIN) $(OBJ)

CC = gcc
BIN = bin/
SRC = src/
INCLUDES = include/
EXEC = k_means
CFLAGS = -O3 -funroll-loops -ftree-vectorize -msse4
LIBS = -lm 
.DEFAULT_GOAL = k_means

k_means: $(SRC)k_means.c $(INCLUDES)k_means.h
	$(CC) $(CFLAGS) $(SRC)k_means.c -o $(BIN)$(EXEC) $(LIBS)
 

clean:
	rm -r bin/*

run:
	./$(BIN)$(EXEC)
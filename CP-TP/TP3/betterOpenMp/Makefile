CC = gcc
BIN = bin/
SRC = src/
INCLUDES = include/
EXEC = k_means
CFLAGS = -O0 -g -fopenmp
#CFLAGS = -O3 -funroll-loops -ftree-vectorize -msse4 

LIBS = -lm 
.DEFAULT_GOAL = k_means
THREADS = 8
CP_CLUSTERS = 4


k_means: $(SRC)k_means.c $(INCLUDES)k_means.h
	$(CC) $(CFLAGS) $(SRC)k_means.c -o $(BIN)$(EXEC) $(LIBS)
 

clean:
	rm -r bin/*

runseq:
	./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS)

runpar:
	./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS) $(THREADS)
CC = nvcc
BIN = bin/
SRC = src/
INCLUDES = include/
EXEC = k_means
#CFLAGS = -O3 -funroll-loops -ftree-vectorize -msse4
#CFLAGS = -O3 -funroll-loops -ftree-vectorize -msse4 
CFLAGS = -O2 -g -std=c++11 -arch=sm_35 -Wno-deprecated-gpu-targets  
LIBS = -lm 
.DEFAULT_GOAL = k_means
THREADS = 4
CP_CLUSTERS = 4


k_means: $(SRC)k_means.cu $(INCLUDES)k_means.h
	$(CC) $(CFLAGS) $(SRC)k_means.cu -o $(BIN)$(EXEC) $(LIBS)
 

clean:
	rm -r bin/*

runseq:
	./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS)

runpar:
	./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS) $(THREADS)
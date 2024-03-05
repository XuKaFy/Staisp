CC = gcc

LIB = libstaisp.c
DIST = libstaisp.ll

run: $(DIST)
	$(CC) -emit-llvm -c $(LIB) -o $(DIST)

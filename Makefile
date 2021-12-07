CXXFLAGS = --std=c99
CROSS_TOOL = 
CC_C = $(CROSS_TOOL)gcc



all: main

main:  main.o filestructure.o *.h
	@echo "Building final files"
	$(CC_C) $(CXXFLAGS) $(DEBUG) main.o filestructure.o -o main
	@echo "Final compilation"

main.o: main.c *.h
	@echo "Compiling main.c"
	$(CC_C) $(CXXFLAGS) -c $(DEBUG) main.c -o main.o
	@echo "Compiled MAIN file"

filestructure.o: filestructure.c *.h
	@echo "Compiling filestructure.h"
	$(CC_C) $(CXXFLAGS) -c $(DEBUG)  filestructure.c -o filestructure.o
	@echo "Compiled filestructure.h"

clean:
	rm -f *.o *.gch ~* a.out main *.db


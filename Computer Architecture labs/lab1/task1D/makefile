all: encoder


encoder: main.o
	gcc -m32 -g -Wall -o encoder main.o 
# Depends on the source and header files
main.o: main.c
	gcc -g -Wall -m32  -c -o main.o main.c 
 
#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o encoder

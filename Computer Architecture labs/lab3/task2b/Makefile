#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: task2b

# Tool invocations
# Executable "bubblesort" depends on the files bubblesort.o
task2b: task2b.o
	gcc -g -m32 -Wall -o task2b task2b.o

# Depends on the source and header files
task2b.o: task2b.c
	gcc -m32 -g -Wall -c -o task2b.o task2b.c 

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o task2b

#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: hexeditplus

# Tool invocations
# Executable "hexeditplus" depends on the files hexeditplus.o
hexeditplus: hexeditplus.o
	gcc -m32 -g -Wall -o hexeditplus hexeditplus.o

# Depends on the source and header files
hexeditplus.o: hexeditplus.c
	gcc -g -Wall -m32  -c -o hexeditplus.o hexeditplus.c 


#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o hexeditplus

#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: flame

# Tool invocations
# Executable "flame" depends on the files flame.o
flame: task2c.o lab4_start.o util.o 
	ld -m elf_i386  lab4_start.o main.o util.o -o flame

# Depends on the source and header files
task2c.o: task2c.c
	gcc -m32 -Wall -ansi -g -c -nostdlib -fno-stack-protector task2c.c -o task2c.o
	
util.o: lab4_util.c
	gcc -m32 -Wall -ansi -g -c -nostdlib -fno-stack-protector lab4_util.c -o util.o
	
lab4_start.o: lab4_start.s
	nasm -f elf lab4_start.s -g -o lab4_start.o

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o flame

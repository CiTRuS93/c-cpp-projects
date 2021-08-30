#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: virus

# Tool invocations
# Executable "task0" depends on the files task0.o
virus: skeleton.o
	ld -m elf_i386 skeleton.o -o virus

# Depends on the source and header files

skeleton.o: skeleton.s
	nasm -f elf skeleton.s -o skeleton.o

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o virus

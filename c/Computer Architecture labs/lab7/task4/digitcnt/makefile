#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: digit_cnt

# Tool invocations
# Executable "hexeditplus" depends on the files hexeditplus.o
digit_cnt: digit_cnt.o
	gcc -m32 -g -Wall -o digit_cnt digit_cnt.o

# Depends on the source and header files
digit_cnt.o: digit_cnt.c
	gcc -g -Wall -m32  -c -o digit_cnt.o digit_cnt.c 


#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o digit_cnt

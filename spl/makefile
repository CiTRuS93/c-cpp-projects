CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++11 -g

BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib

LIBRARIES	:=
EXECUTABLE	:= rest


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)
$(EXECUTABLE):$(BIN)/*.o
	$(CXX) -o $(BIN)/$(EXECUTABLE) $(BIN)/*.o 
clean:
	-rm $(BIN)/*

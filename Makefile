PROJECT = ./bin/libpigeon.a

CPP = $(wildcard ./source/*.cpp)

INC = -I ./include -I /usr/include/ -I /usr/local/include/rapidjson/

CXX = clang++

FLAGS = -Wall -Wextra -pedantic -pedantic-errors -std=c++14 -g3 $(INC)

OBJ = $(CPP:.cpp=.o)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $^ -o $@

all: cleanbin build clean install

build: $(OBJ)
	ar -cvq $(PROJECT) $(OBJ)

cleanbin:
	#rm $(PROJECT)

clean:
	rm ./source/*.o
install:
	sudo cp $(PROJECT) /usr/local/lib
	sudo cp ./include/* /usr/local/include/pigeon
    
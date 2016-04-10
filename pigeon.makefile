PROJECT = ./bin/libpigeon.a

CPP = $(wildcard ./source/*.cpp)

INC = -I ./include -I /usr/include/ -I /usr/local/include/rapidjson/

CXX = clang++

FLAGS = -Wall -Wextra -pedantic-errors -std=c++14 -O3 $(INC)

OBJ = $(CPP:.cpp=.o)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $^ -o $@

all: $(OBJ)
	rm $(PROJECT)
	ar -cvq $(PROJECT) $(OBJ)
	rm ./source/*.o


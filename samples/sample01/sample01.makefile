PROJECT = ./service/bin/sample01

CPP = $(wildcard ./service/source/*.cpp)

INC = -I ./service/include/ -I /usr/local/include/pigeon
LIBS = pthread pigeon uv z http_parser mongoclient boost_system boost_filesystem boost_thread boost_regex

CXX = g++

FLAGS = -Wall -std=c++14 -O3 -g0 $(INC)

OBJ = $(CPP:.cpp=.o)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $^ -o $@

all: $(OBJ)
	$(CXX) $(FLAGS) $(OBJ) -o $(PROJECT) $(addprefix -l, $(LIBS))
	./service/bin/sample01
PROJECT = ./bin/libpigeon.a

CPP = $(wildcard ./source/*.cpp)

INC = -I ./include \
    -I ./deps/libuv/include \
    -I ./deps/rapidjson/include/ \
    -I ./deps/http-parser/ \
    -I ./deps/zlib/

#CXX = g++

FLAGS = -Wall -Wextra -pedantic -pedantic -std=c++11 -O3 $(INC)

OBJ = $(CPP:.cpp=.o)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $^ -o $@

all: ./deps/rapidjson ./deps/zlib ./deps/libuv ./deps/http-parser cleanbin build clean

./deps/http-parser:
	git clone --depth 1 git://github.com/joyent/http-parser.git ./deps/http-parser

./deps/libuv:
	git clone --depth 1 git://github.com/libuv/libuv.git ./deps/libuv

./deps/zlib:
	git clone --depth 1 https://github.com/madler/zlib.git ./deps/zlib

./deps/rapidjson:
	git clone --depth 1 https://github.com/miloyip/rapidjson.git ./deps/rapidjson

build: $(OBJ)
	ar -cvq $(PROJECT) $(OBJ)

test:

cleanbin:
	#rm $(PROJECT)

clean:
	rm ./source/*.o
install:
	sudo cp $(PROJECT) /usr/local/lib
	sudo cp ./include/* /usr/local/include/pigeon

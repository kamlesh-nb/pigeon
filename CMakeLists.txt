cmake_minimum_required(VERSION 3.2)
project(pigeon)
FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(LIBUV NAMES uv paths /usr/lib)
FIND_LIBRARY(LIBHP NAMES http_parser paths /usr/lib)
FIND_LIBRARY(LIBZ NAMES z paths /usr/lib)

FIND_PACKAGE(Boost COMPONENTS system filesystem thread REQUIRED)
FIND_LIBRARY(LIBMONGOCLIENT NAMES mongoclient PATHS /usr/local/lib)


include_directories("${pigeon_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES
        include/file_info.h
        include/key_value_pair.h
        include/http_handler_base.h
        source/logger.cpp include/logger.h
        source/http_util.cpp include/http_util.h
        source/http_msg.cpp include/http_msg.h
        source/http_handler.cpp include/http_handler.h
        source/stop_watch.cpp include/stop_watch.h
        source/md5.cpp include/md5.h
        source/server.cpp include/server.h
        include/http_context.h include/http_context.h
        source/app.cpp include/app.h
        source/http_handler.cpp include/http_handler.h
        source/cache.cpp include/cache.h
        source/settings.cpp include/settings.h
        source/rest_handlers.cpp include/rest_handlers.h)

add_library(pigeon ${SOURCE_FILES})
target_link_libraries (pigeon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (pigeon ${LIBZ})
target_link_libraries (pigeon ${LIBUV})
target_link_libraries (pigeon ${LIBHP})


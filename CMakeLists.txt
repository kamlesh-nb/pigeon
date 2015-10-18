cmake_minimum_required(VERSION 3.2)
project(pigeon)
FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(LIBUV NAMES uv paths /usr/lib)
FIND_LIBRARY(LIBHP NAMES http_parser paths /usr/lib)
FIND_LIBRARY(LIBZ NAMES z paths /usr/lib)
FIND_LIBRARY(LIBSSL NAMES ssl paths /usr/lib/x86_64-linux-gnu)
FIND_LIBRARY(LIBCRYPTO NAMES crypto++ paths /usr/lib)
include_directories("${pigeon_SOURCE_DIR}/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp
        source/config.cpp include/config.h
        source/file_cache.cpp include/file_cache.h
        include/file_info.h
        include/key_value_pair.h
        include/api_handler_base.h
        include/uv_globals.h
        source/logger.cpp include/logger.h
        source/app_constants.cpp include/app_constants.h
        source/http_msg.cpp include/http_msg.h
        source/request_handler.cpp include/request_handler.h
        source/stop_watch.cpp include/stop_watch.h
        source/md5.cpp include/md5.h
        include/server.h
        app_service.cpp app_service.h source/server.cpp)

add_executable(pigeon ${SOURCE_FILES})
target_link_libraries (pigeon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (pigeon ${LIBZ})
target_link_libraries (pigeon ${LIBUV})
target_link_libraries (pigeon ${LIBHP})
target_link_libraries (pigeon ${LIBSSL})
target_link_libraries (pigeon ${LIBCRYPTO})

cmake_minimum_required(VERSION 3.2)
project(pigeon)
FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(LIBHP NAMES http_parser paths /usr/lib)
FIND_LIBRARY(LIBZ NAMES z paths /usr/lib)
FIND_LIBRARY(LIBUV NAMES uv paths /usr/local/lib)

include_directories("${pigeon_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp
        include/http_context.h
        include/file_info.h
        include/key_value_pair.h
        include/http_handler_base.h
        include/http_filter_base.h
        source/logger.cpp include/logger.h
        source/http_util.cpp include/http_util.h
        source/http_msg.cpp include/http_msg.h
        source/resource_handler.cpp include/resource_handler.h
        source/stop_watch.cpp include/stop_watch.h
        source/md5.cpp include/md5.h
        source/resource_handler.cpp include/resource_handler.h
        source/cache.cpp include/cache.h
        source/settings.cpp include/settings.h
        source/server.cpp include/server.h
        source/http_filters.cpp include/http_filters.h
        source/http_handlers.cpp include/http_handlers.h include/http_session.h)

#add_library(pigeon ${SOURCE_FILES})
add_executable(pigeon ${SOURCE_FILES})

target_link_libraries (pigeon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (pigeon ${LIBUV})
target_link_libraries (pigeon ${LIBZ})
target_link_libraries (pigeon ${LIBHP})



cmake_minimum_required(VERSION 3.3)
project(pigeon)
FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(LIBHP NAMES http_parser paths /usr/lib)
FIND_LIBRARY(LIBZ NAMES z paths /usr/lib)
FIND_LIBRARY(LIBFOLLY NAMES folly paths /usr/local/lib)
FIND_LIBRARY(LIBGLOG NAMES glog paths /usr/lib/x86_64-linux-gnu)
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

include_directories("${pigeon_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp
        include/file_info.h
        include/key_value_pair.h
        include/http_handler_base.h
        include/http_filter_base.h
        include/http_connection.h
        source/logger.cpp include/logger.h
        source/http_util.cpp include/http_util.h
        source/http_msg.cpp include/http_msg.h
        source/resource_handler.cpp include/resource_handler.h
        source/stop_watch.cpp include/stop_watch.h
        source/md5.cpp include/md5.h
        source/resource_handler.cpp include/resource_handler.h
        source/cache.cpp include/cache.h
        source/settings.cpp include/settings.h
        source/tcp_server.cpp include/tcp_server.h
        source/io_contexts.cpp include/io_contexts.h
        source/http_connection.cpp include/http_connection.h
        source/http_msg_parser.cpp include/http_msg_parser.h
        source/http_connection_base.cpp include/http_connection_base.h
        source/http_filters.cpp include/http_filters.h
        source/http_handlers.cpp include/http_handlers.h)

#add_library(pigeon ${SOURCE_FILES})
add_executable(pigeon ${SOURCE_FILES})

target_link_libraries (pigeon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (pigeon ${LIBFOLLY})
target_link_libraries (pigeon ${LIBGLOG})
target_link_libraries (pigeon ${LIBZ})
target_link_libraries (pigeon ${LIBHP})
target_link_libraries(pigeon
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY})


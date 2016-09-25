cmake_minimum_required(VERSION 3.5)
project(pigeon)

FIND_PACKAGE(Threads REQUIRED)
FIND_LIBRARY(LIBUV NAMES uv paths /usr/local/lib)
FIND_LIBRARY(LIBHP NAMES http_parser paths /usr/local/lib)
FIND_LIBRARY(LIBZ NAMES z paths /usr/local/lib)
FIND_LIBRARY(LIBSSL NAMES ssl paths /usr/lib)
FIND_LIBRARY(LIBCRYPTO NAMES crypto paths /usr/lib)

include_directories("/home/kamlesh/kdevelop/Projects/pigeon/include")
include_directories("/home/kamlesh/kdevelop/Projects/pigeon/include/net")
include_directories("/home/kamlesh/kdevelop/Projects/pigeon/include/data")
include_directories("/home/kamlesh/kdevelop/Projects/pigeon/src/net/impl")
include_directories("usr/local/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp
        include/net/FileInfo.h
        include/net/HttpContext.h
        src/net/HttpServer.cpp include/net/HttpServer.h
        src/net/MultiPartParser.cpp include/net/MultiPartParser.h
        src/net/HttpMessage.cpp include/net/HttpMessage.h
        src/net/md5.cpp include/net/md5.h
        src/StopWatch.cpp include/StopWatch.h
        src/net/StringBuilder.cpp include/net/StringBuilder.h
        src/net/impl/MessageParser.ipp
        src/net/impl/HandleDispatcher.ipp
        src/net/impl/Listener.ipp
        src/net/impl/TcpListener.ipp
        src/net/impl/TlsTcpListener.ipp
        src/net/HttpUtil.cpp include/net/HttpUtil.h
        src/net/FileCache.cpp include/net/FileCache.h
        src/Settings.cpp include/Settings.h
        src/Logger.cpp include/Logger.h
        src/data/rdb/ResultSet.cpp include/data/rdb/ResultSet.h
        include/data/rdb/protocol_defs.h
        src/AppService.cpp include/AppService.h
        include/data/IDbConnection.h
        src/data/rdb/RdbConnection.cpp include/data/rdb/RdbConnection.h
        src/net/RequestHandler.cpp include/net/RequestHandler.h
        src/data/rdb/R.cpp include/data/rdb/R.h)

add_executable(pigeon ${SOURCE_FILES})
target_link_libraries (pigeon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (pigeon ${LIBZ})
target_link_libraries (pigeon ${LIBUV})
target_link_libraries (pigeon ${LIBHP})
target_link_libraries (pigeon ${LIBSSL})
target_link_libraries (pigeon ${LIBCRYPTO})

install(TARGETS pigeon RUNTIME DESTINATION bin)

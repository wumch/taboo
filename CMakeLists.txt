cmake_minimum_required(VERSION 3.3)
project(taboo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/websocket.cpp src/node.hpp src/trie.hpp)
add_executable(taboo ${SOURCE_FILES})

include_directories("/data/code/wumch/websocketpp")

cmake_minimum_required(VERSION 2.8)
project(taboo)

include_directories(./libs ./libs/websocketpp ./libs/rapidjson/include)

set(SOURCE_FILES src/taboo.cpp src/Aside.cpp src/Filter.cpp src/Config.cpp
    src/Seeker.hpp src/Aside.hpp src/Farm.hpp src/Trie.hpp src/Filter.hpp src/Item.hpp src/Config.hpp src/predef.hpp)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -ferror-limit=5)
#add_compile_options(-std=c++11)

link_libraries(boost_system boost_chrono boost_program_options boost_filesystem)

add_executable(taboo ${SOURCE_FILES})

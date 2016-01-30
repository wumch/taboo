
cmake_minimum_required(VERSION 3.3)
project(taboo)

include_directories(./libs ./libs/websocketpp ./libs/rapidjson/include)

set(SOURCE_FILES src/taboo.cpp src/aside.cpp src/aside.hpp
    src/seeker.hpp src/item.hpp src/attr.hpp src/predef.hpp)

set(CMAKE_CXX_COMPILER clang++)
#add_compile_options(-std=c++11)

link_libraries(boost_system boost_chrono)

add_executable(taboo ${SOURCE_FILES})

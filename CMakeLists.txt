
cmake_minimum_required(VERSION 2.8)
project(taboo)

include_directories(./libs ./libs/websocketpp ./libs/rapidjson/include)

set(SOURCE_FILES    src/taboo.cpp 
    src/Manager.cpp    src/Router.cpp 
    src/Keeper.cpp    src/Item.cpp 
    src/Aside.cpp    src/Config.cpp

    src/Manager.hpp    src/Router.hpp 
    src/Seeker.hpp    src/Query.hpp
    src/Farm.hpp    src/Trie.hpp    src/Filter.hpp    src/Item.hpp
    src/manage/StatusHandler.hpp    src/manage/AttachHandler.hpp    src/manage/DetachHandler.hpp
    src/manage/BaseHandler.hpp    src/BaseHandler.hpp
    src/Aside.hpp    src/Config.hpp    src/predef.hpp)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -ferror-limit=5 -DCS_DEBUG=1 -O0 -ggdb -Wall -Wno-unused-local-typedef")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -DNDEBUG -Wall -Wno-unused-local-typedef")
#add_compile_options(-std=c++11)

link_libraries(boost_system boost_chrono boost_program_options boost_filesystem boost_thread crypto microhttpd glog)

add_executable(taboo ${SOURCE_FILES})

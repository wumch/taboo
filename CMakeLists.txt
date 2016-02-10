
cmake_minimum_required(VERSION 2.8)
project(taboo)

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_lower)
if (cmake_build_type_lower MATCHES "debug")
    
    if (DEFINED CS_DEBUG)
    else()
        set(CS_DEBUG 2)
    endif()
    
    find_program(ccache_path ccache)
    find_program(clang_path clang++)
    if (clang_path)
        set(CMAKE_CXX_COMPILER ${clang_path})
    endif()
    
    add_compile_options(-O0 -g3 -DCS_DEBUG=${CS_DEBUG} -ferror-limit=5)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-O0 -Wl,-as-needed")
    
else()
    add_compile_options(-O3 -DNDEBUG)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-O2 -Wl,-as-needed")
endif()

add_compile_options(-pthread -fomit-frame-pointer -pipe -fno-strict-aliasing -Wall -Wno-unused-local-typedef)

include_directories(libs libs/websocketpp libs/rapidjson/include)

link_libraries(boost_system boost_chrono boost_program_options boost_filesystem boost_thread crypto microhttpd glog)

file(GLOB_RECURSE SOURCE_FILES src/*.?pp)
add_executable(taboo ${SOURCE_FILES})

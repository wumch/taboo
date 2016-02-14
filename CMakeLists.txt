
cmake_minimum_required(VERSION 2.8)
project(taboo)

add_compile_options(-pthread -fomit-frame-pointer -pipe -fno-strict-aliasing -Wall)
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
        add_compile_options(-Wno-unused-local-typedef)
    else()
        add_compile_options(-Wno-unused-local-typedefs -Wno-enum-compare)
    endif()
    
    add_compile_options(-O0 -g3 -DCS_DEBUG=${CS_DEBUG} -ferror-limit=5)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-O0 -Wl,-as-needed")
    
else()
    add_compile_options(-O3 -DNDEBUG -finline-functions
        -Wno-unused-local-typedefs -Wno-enum-compare)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-O3 -Wl,-as-needed")
endif()

include_directories(libs libs/websocketpp libs/rapidjson/include)

link_libraries(boost_system boost_chrono boost_program_options boost_filesystem boost_thread 
    crypto microhttpd glog)

# todo: should link staticlly
find_library(lib_malloc jemalloc)
if (lib_malloc)
    link_libraries(jemalloc)
else()
    find_library(lib_malloc tcmalloc)
    if (lib_malloc)
        link_libraries(tcmalloc)
    endif()
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.?pp)
add_executable(taboo ${SOURCE_FILES})

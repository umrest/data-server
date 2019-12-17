cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project( data-server )

file(GLOB data-server_SRC
    "include/*.h"
    "src/*.cpp"
)

find_package( Boost COMPONENTS filesystem system REQUIRED )
find_package (Threads REQUIRED)

include_directories(
    include
    ${BOOST_INCLUDE_DIRS}
)

add_executable( data-server ${data-server_SRC} )

target_link_libraries( data-server
${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} )
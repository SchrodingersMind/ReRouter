cmake_minimum_required(VERSION 3.21)
project(ReRouter C)

set(CMAKE_C_STANDARD 99)
add_link_options("-pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread" )

add_executable(ReRouter main.c mine_sock.c include/misc.h include/mine_sock.h)

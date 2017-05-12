cmake_minimum_required(VERSION 2.8.11)

project(leaderboard C CXX)

# set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -g -m64 -Wall -Werror")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

list(APPEND SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_library(LIB_AMQPCPP amqpcpp)
list(APPEND LIBS ${LIB_AMQPCPP})

foreach (_dir ${SRC_DIRS})
    file(GLOB_RECURSE _src ${_dir}/*.cpp)
    list(APPEND SRC_ALL ${_src})
endforeach()

add_executable(leaderboard.out ${SRC_ALL})
target_link_libraries(leaderboard.out ${LIBS})
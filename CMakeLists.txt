cmake_minimum_required(VERSION 2.8.11)

project(leaderboard C CXX)

# set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O2 -g -m64 -Wall -Werror")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(/usr/local/include/mongocxx/v_noabi)
include_directories(/usr/local/include/libmongoc-1.0)
include_directories(/usr/local/include/bsoncxx/v_noabi)
include_directories(/usr/local/include/libbson-1.0)

find_library(LIB_AMQPCPP amqpcpp)
list(APPEND LIBS ${LIB_AMQPCPP})
find_library(LIB_CONFIG config++)
list(APPEND LIBS ${LIB_CONFIG})
find_library(LIB_MONGO_CXX mongocxx)
list(APPEND LIBS ${LIB_MONGO_CXX})
find_library(LIB_BSON_CXX bsoncxx)
list(APPEND LIBS ${LIB_BSON_CXX})

# do not add the whole src directory
list(APPEND SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/app
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/db
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/common
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/rabbitmq)

function(my_add_executable EXECUTABLE SRC_DIRS LIBS)
    foreach (_dir ${SRC_DIRS})
        file(GLOB_RECURSE _src ${_dir}/*.cpp)
        list(APPEND SRC_ALL ${_src})
    endforeach()

    add_executable(${EXECUTABLE} ${SRC_ALL})
    target_link_libraries(${EXECUTABLE} ${LIBS})
endfunction(my_add_executable)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/leaderboard_app)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/traffic_generator)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

file(GLOB CONFIGURATION_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cfg")
file(COPY ${CONFIGURATION_FILES} DESTINATION ${CMAKE_BINARY_DIR})
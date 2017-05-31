cmake_minimum_required(VERSION 2.8.11)

project(traffic_generator C CXX)

set(MY_SRC_DIRS ${SRC_DIRS})
list(APPEND MY_SRC_DIRS ${CMAKE_SOURCE_DIR}/external/fantasyname)
list(APPEND MY_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

my_add_executable(${PROJECT_NAME}.out "${MY_SRC_DIRS}" "${LIBS}")
cmake_minimum_required(VERSION 2.8.11)

project(leaderboard_app C CXX)

set(MY_SRC_DIRS ${SRC_DIRS})
list(APPEND MY_SRC_DIRS ${CMAKE_SOURCE_DIR}/src/app
                        ${CMAKE_SOURCE_DIR}/src/db
                        ${CMAKE_CURRENT_SOURCE_DIR})

set(MY_LIBS ${LIBS})
list(APPEND MY_LIBS ${LIB_MONGO_CXX})
list(APPEND MY_LIBS ${LIB_BSON_CXX})

my_add_executable(${PROJECT_NAME}.out "${MY_SRC_DIRS}" "${MY_LIBS}")
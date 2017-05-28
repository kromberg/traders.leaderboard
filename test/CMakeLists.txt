cmake_minimum_required(VERSION 2.8.11)

project(leaderboard_test C CXX)

set(MY_SRC_DIRS ${SRC_DIRS})
list(APPEND MY_SRC_DIRS ${CMAKE_SOURCE_DIR}/src/app
                        ${CMAKE_SOURCE_DIR}/src/db
                        ${CMAKE_CURRENT_SOURCE_DIR})

set(MY_LIBS ${LIBS})
list(APPEND MY_LIBS ${LIB_MONGO_CXX})
list(APPEND MY_LIBS ${LIB_BSON_CXX})
find_library(LIB_GTEST gtest)
list(APPEND MY_LIBS ${LIB_GTEST})

my_add_executable(${PROJECT_NAME}.out "${MY_SRC_DIRS}" "${MY_LIBS}")
my_add_executable(${PROJECT_NAME}.out.coverage "${MY_SRC_DIRS}" "${MY_LIBS}")
set_target_properties(${PROJECT_NAME}.out.coverage PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -fPIC")
set_target_properties(${PROJECT_NAME}.out.coverage PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage -fPIC")

set(COVERAGE_FILE ${CMAKE_BINARY_DIR}/coverage.info)
set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)

add_custom_command(OUTPUT ${COVERAGE_FILE}
    COMMAND lcov --capture --directory . --output-file ${COVERAGE_FILE}
    COMMAND lcov --remove ${COVERAGE_FILE} \"/usr*\" \"*${CMAKE_CURRENT_SOURCE_DIR}*\" \"v1/*\" -o ${COVERAGE_FILE})

add_custom_target(coverage
    COMMAND genhtml ${COVERAGE_FILE} --output-directory ${COVERAGE_DIR}
    DEPENDS ${COVERAGE_FILE})

add_custom_target(clean-coverage 
    COMMAND find ${CMAKE_BINARY_DIR} -name \"*.gcda\" -print0 | xargs -0 rm
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove ${COVERAGE_FILE})
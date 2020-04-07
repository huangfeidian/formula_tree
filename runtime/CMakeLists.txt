cmake_minimum_required(VERSION 3.2)

PROJECT(formula_tree_runtime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
FILE(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp)
find_package(nlohmann_json CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${nlohmann_json_include_dirs})
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

find_package(any_container CONFIG REQUIRED)

add_library(formula_tree_runtime ${RUNTIME_SRCS})
target_link_libraries(formula_tree_runtime nlohmann_json::nlohmann_json any_container)

add_executable(print_formula ${CMAKE_SOURCE_DIR}/test/print_formula.cpp)
target_link_libraries(print_formula formula_tree_runtime)

add_executable(print_update ${CMAKE_SOURCE_DIR}/test/print_update.cpp)
target_link_libraries(print_update formula_tree_runtime)
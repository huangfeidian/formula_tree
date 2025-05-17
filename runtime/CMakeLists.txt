cmake_minimum_required(VERSION 3.10)

PROJECT(formula_tree_runtime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
FILE(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
find_package(nlohmann_json CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${nlohmann_json_include_dirs})
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

find_package(any_container CONFIG REQUIRED)

add_library(formula_tree_runtime ${RUNTIME_SRCS})
target_link_libraries(formula_tree_runtime nlohmann_json::nlohmann_json any_container magic_enum::magic_enum)
target_include_directories(formula_tree_runtime INTERFACE $<INSTALL_INTERFACE:include>)
add_executable(print_formula ${CMAKE_CURRENT_SOURCE_DIR}/test/print_formula.cpp)
target_link_libraries(print_formula formula_tree_runtime)

add_executable(print_update ${CMAKE_CURRENT_SOURCE_DIR}/test/print_update.cpp)
target_link_libraries(print_update formula_tree_runtime)

add_executable(parse_formula ${CMAKE_CURRENT_SOURCE_DIR}/test/parse_formula.cpp)
target_link_libraries(parse_formula formula_tree_runtime)

if(IS_TOPLEVEL_PROJECT)
foreach(p LIB BIN INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		string(TOLOWER ${p} lower_p)
        set(${var} "${CMAKE_INSTALL_PREFIX}/${lower_p}")
	endif()
endforeach()

export(TARGETS ${PROJECT_NAME}
     FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)

file(WRITE
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    "include(\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake)\n")

install(FILES
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
   ARCHIVE  DESTINATION lib
   LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/${PROJECT_NAME})

install(EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
endif(IS_TOPLEVEL_PROJECT)
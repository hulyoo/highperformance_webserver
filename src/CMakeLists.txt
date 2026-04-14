cmake_minimum_required(VERSION 3.10)
file(GLOB_RECURSE SRC_LIST
    LIST_DIRECTORIES false 
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/*")

# add_compile_definitions(_DEFAULT_SOURCE)
add_executable(server ${SRC_LIST})    

set(INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)
target_include_directories(server PRIVATE ${INCLUDE_PATH})
set_target_properties(server PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")           

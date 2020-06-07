cmake_minimum_required(VERSION 3.12)
    
project(Swoosh-Demo)
set (CMAKE_CXX_STANDARD 17)

execute_process(COMMAND git submodule update --init -- extern/SFML
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(extern/SFML)

file(GLOB_RECURSE Files CONFIGURE_DEPENDS
        "ExampleDemo/*.h"
        "ExampleDemo/*.cpp"
        )

add_executable(ExampleDemo ExampleDemo/Demo.cpp ${Files})

target_include_directories(ExampleDemo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/Segues")

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${Files})

target_link_libraries(ExampleDemo sfml-graphics sfml-audio sfml-network sfml-system sfml-window)

set_target_properties(ExampleDemo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Compiler.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostBuild.cmake)
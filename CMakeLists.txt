cmake_minimum_required(VERSION 3.12)
    
project(Swoosh-Demo)

set (CMAKE_CXX_STANDARD 17)

execute_process(COMMAND git submodule update --init -- ExampleDemo/extern/SFML
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake)

add_subdirectory(ExampleDemo/extern/SFML)

file(GLOB SwooshFiles CONFIGURE_DEPENDS
        "ExampleDemo/*.h"
        "ExampleDemo/Scenes/*.h"
        "ExampleDemo/*.cpp"
        "ExampleDemp/Scenes/*.cpp"
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(ExampleDemo ExampleDemo/Demo.cpp ${SwooshFiles})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SwooshFiles})

target_link_libraries(ExampleDemo sfml-graphics sfml-audio sfml-network sfml-system sfml-window)

set_target_properties(ExampleDemo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Compiler.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostBuild.cmake)
# Should contain compiler-dependent configuration for the project
# This file is included after project and after target declarations in parent CMakeLists.

if(MSVC)

    set_property(TARGET Demo 
                PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>")

endif()

if(NOT MSVC)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-value")

endif()

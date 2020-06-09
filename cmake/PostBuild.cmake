# Should define post-build steps (e.g. resource, dll copying) for the target
# This file is included at the end of the parent CMakeLists.txt

if(WIN32)

    ADD_CUSTOM_COMMAND(
        TARGET ExampleDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different
        "${PROJECT_SOURCE_DIR}/ExampleDemo/extern/SFML/extlibs/bin/x64/openal32.dll"
        
        "$<TARGET_FILE_DIR:ExampleDemo>/"
        
        COMMENT "Copying openal32.dll\n"
    )

endif()

if(WIN32 OR UNIX)

    ADD_CUSTOM_COMMAND(
        TARGET ExampleDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_directory 
        
        "${PROJECT_SOURCE_DIR}/ExampleDemo/resources"
        
        "$<TARGET_FILE_DIR:ExampleDemo>/resources"
        
        COMMENT "Copying resources\n"
    )
endif()

if(BUILD_SHARED_LIBS)

    ADD_CUSTOM_COMMAND(
        TARGET ExampleDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_directory
        
        "$<TARGET_FILE_DIR:sfml-graphics>"
        "$<TARGET_FILE_DIR:sfml-audio>"
        "$<TARGET_FILE_DIR:sfml-network>"
        "$<TARGET_FILE_DIR:sfml-system>"
        "$<TARGET_FILE_DIR:sfml-window>"
        
        "$<TARGET_FILE_DIR:ExampleDemo>"
        
        COMMENT "Copying shared libraries\n"
    )

endif()
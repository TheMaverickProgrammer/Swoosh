# Should define post-build steps (e.g. resource, dll copying) for the target
# This file is included at the end of the parent CMakeLists.txt

if(WIN32)

    ADD_CUSTOM_COMMAND(
        TARGET ExampleDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different 
        
        "${PROJECT_SOURCE_DIR}/extern/lib/win32/openal32.dll"
        
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
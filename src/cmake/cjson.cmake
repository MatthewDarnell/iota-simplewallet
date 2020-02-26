
#cJSON
ExternalProject_Add(json
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON
        GIT_TAG 2955fe5ec49a672848a323a7c943c9525d16c83f
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS 1
        PATCH_COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/deps/include/cjson/
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.c ${CMAKE_SOURCE_DIR}/deps/cJSON.c
        UPDATE_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.h ${CMAKE_SOURCE_DIR}/deps/include/cjson/
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.h ${CMAKE_SOURCE_DIR}/deps/
        )

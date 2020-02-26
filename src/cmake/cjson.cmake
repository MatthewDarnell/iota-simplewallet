
#cJSON

set(CJSON_PATH "${CMAKE_CURRENT_BINARY_DIR}/json-prefix/src/json")
ExternalProject_Add(json
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON
        GIT_TAG 2955fe5ec49a672848a323a7c943c9525d16c83f
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS 1
        PATCH_COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/deps/include/cjson/
        CONFIGURE_COMMAND ${CMAKE_C_COMPILER} -c ${CJSON_PATH}/cJSON.c
        UPDATE_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.h ${CMAKE_SOURCE_DIR}/deps/include/cjson/
        BUILD_COMMAND ${CMAKE_AR} rcs ${CMAKE_SOURCE_DIR}/deps/lib/libcJSON.a ${CJSON_PATH}/cJSON.o
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.h ${CMAKE_SOURCE_DIR}/deps/
        )

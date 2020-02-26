#iota.c
#ugly hack for mingw
set(IOTA_PATH "${CMAKE_CURRENT_BINARY_DIR}/iota.c-library-prefix/src/iota.c-library")
set(IOTA_PATH_BUILD "${CMAKE_CURRENT_BINARY_DIR}/iota.c-library-prefix/src/iota.c-library-build")

if (WIN32)
    file(MAKE_DIRECTORY "${IOTA_PATH_BUILD}/mbedtls/src/ext_mbedtls-build/include/mbedtls")
    file(MAKE_DIRECTORY "${IOTA_PATH_BUILD}/mbedtls/src/ext_mbedtls-build/scripts")
endif (WIN32)


ExternalProject_Add(iota.c-library
        GIT_REPOSITORY https://github.com/iotaledger/iota.c
        GIT_TAG 22b52d7c4333d6828ab48b811807be8081fbf15c
        UPDATE_COMMAND ""
        CMAKE_ARGS -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_LINK_LIBRARY_FLAG="${CMAKE_LINK_LIBRARY_FLAG} -lm" -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
        )



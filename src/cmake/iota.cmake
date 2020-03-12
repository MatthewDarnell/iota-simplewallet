#iota.c
#ugly hack for mingw
set(IOTA_PATH "${CMAKE_CURRENT_BINARY_DIR}/iota.c-library-prefix/src/iota.c-library")
set(IOTA_PATH_BUILD "${CMAKE_CURRENT_BINARY_DIR}/iota.c-library-prefix/src/iota.c-library-build")

if (WIN32)
    file(MAKE_DIRECTORY "${IOTA_PATH_BUILD}/mbedtls/src/ext_mbedtls-build/include/mbedtls")
    file(MAKE_DIRECTORY "${IOTA_PATH_BUILD}/mbedtls/src/ext_mbedtls-build/scripts")
endif (WIN32)

if (WIN32)
    set(LINK_FLAGS " -fPIC -Wl,--export-all-symbols")
    ExternalProject_Add(iota.c-library
            GIT_REPOSITORY https://github.com/iotaledger/iota.c
            GIT_TAG d632a84272dab322e9fbcbe71a5e6ac7e8168900
            UPDATE_COMMAND ""
            CMAKE_ARGS -DCMAKE_BUILD_TYPE="Debug" -DBUILD_SHARED_LIBS=OFF -DCMAKE_LINK_LIBRARY_FLAG=" ${CMAKE_LINK_LIBRARY_FLAG} -fPIC -lm wldap32 ws2_32 wsock32 crypt32" -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
            )
else ()
    ExternalProject_Add(iota.c-library
            GIT_REPOSITORY https://github.com/iotaledger/iota.c
            GIT_TAG d632a84272dab322e9fbcbe71a5e6ac7e8168900
            BUILD_COMMAND COMMAND ${CMAKE_COMMAND} -E env CFLAGS=-fPIC make
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS=-fPIC -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
            )
endif()

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
            GIT_REPOSITORY https://github.com/MatthewDarnell/iota.c
            GIT_TAG 46ef92fa5f1ee8a08d265a5adbc7332932f533b4
            UPDATE_COMMAND ""
            CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DBUILD_SHARED_LIBS=OFF
                -DCMAKE_LINK_LIBRARY_FLAG=" ${CMAKE_LINK_LIBRARY_FLAG} -fPIC -lm wldap32 ws2_32 wsock32 crypt32"
                -DCMAKE_INSTALL_PREFIX=${DEPS_DIR}
            )
else ()
    ExternalProject_Add(iota.c-library
            GIT_REPOSITORY https://github.com/MatthewDarnell/iota.c.git
            GIT_TAG 46ef92fa5f1ee8a08d265a5adbc7332932f533b4
            CMAKE_ARGS 
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
		-DBUILD_SHARED_LIBS=OFF 
		-DCMAKE_POSITION_INDEPENDENT_CODE=On
		-DCMAKE_INSTALL_PREFIX=${DEPS_DIR}
            )
endif()

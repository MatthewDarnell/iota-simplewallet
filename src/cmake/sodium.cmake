

#libsodium
if (WIN32)
    if(DEFINED x64)
        set(CRYPTO_FOLDER libsodium-win64)
    else()
        set(CRYPTO_FOLDER libsodium-win32)
    endif()
    set(SODIUM_STATIC 1)
    set(SODIUM_EXPORT)
    ExternalProject_Add(libsodium
            URL "https://download.libsodium.org/libsodium/releases/libsodium-1.0.18-mingw.tar.gz"
            URL_HASH SHA1=9e7143e7f0e4232bd6b867cf37e7995165a220ae
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRYPTO_FOLDER}/include ${DEPS_DIR}/include
            BUILD_COMMAND ""
            BUILD_ALWAYS 1
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRYPTO_FOLDER}/lib ${DEPS_DIR}/lib
            )
else ()
    ExternalProject_Add(libsodium
            URL "https://github.com/jedisct1/libsodium/releases/download/1.0.18-RELEASE/libsodium-1.0.18.tar.gz"
            URL_HASH SHA1=795b73e3f92a362fabee238a71735579bf46bb97
            BUILD_IN_SOURCE 1
            BUILD_ALWAYS 1
            CMAKE_ARGS -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -fPIC"
            CONFIGURE_COMMAND ./configure --enable-pic --enable-shared --prefix=${DEPS_DIR}
            BUILD_COMMAND make
            )
endif (WIN32)



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
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRYPTO_FOLDER}/include ${CMAKE_SOURCE_DIR}/deps/include
            BUILD_COMMAND ""
            BUILD_ALWAYS 1
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRYPTO_FOLDER}/lib ${CMAKE_SOURCE_DIR}/deps/lib
            )
endif (WIN32)
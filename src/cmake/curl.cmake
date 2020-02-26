

#cURL
if (WIN32)

    ExternalProject_Add(zlib
            URL  https://bintray.com/vszakats/generic/download_file?file_path=zlib-1.2.11-win64-mingw.zip
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy libz.a ${CMAKE_SOURCE_DIR}/deps/lib
            BUILD_IN_SOURCE 1
            BUILD_COMMAND ""
            UPDATE_COMMAND ${CMAKE_COMMAND} -E copy zconf.h ${CMAKE_SOURCE_DIR}/deps/include
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy zlib.h ${CMAKE_SOURCE_DIR}/deps/include
            )

    ExternalProject_Add(brotli
            URL  https://bintray.com/vszakats/generic/download_file?file_path=brotli-1.0.7-win64-mingw.zip
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${CMAKE_SOURCE_DIR}/deps/lib
            BUILD_IN_SOURCE 1
            BUILD_COMMAND ""
            UPDATE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory include ${CMAKE_SOURCE_DIR}/deps/include
            )

    ExternalProject_Add(nghttp2
            URL  https://bintray.com/vszakats/generic/download_file?file_path=nghttp2-1.40.0-win64-mingw.zip
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${CMAKE_SOURCE_DIR}/deps/lib
            BUILD_IN_SOURCE 1
            BUILD_COMMAND ""
            UPDATE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory include ${CMAKE_SOURCE_DIR}/deps/include
            )

    ExternalProject_Add(libssh2
            GIT_REPOSITORY https://github.com/libssh2/libssh2
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
            UPDATE_COMMAND ""
            )

    ExternalProject_Add(openssl
            URL  https://bintray.com/vszakats/generic/download_file?file_path=openssl-1.1.1d-win64-mingw.zip
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${CMAKE_SOURCE_DIR}/deps/lib
            BUILD_IN_SOURCE 1
            BUILD_COMMAND ""
            UPDATE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory include ${CMAKE_SOURCE_DIR}/deps/include
            )

    ExternalProject_Add(curl
            URL  https://curl.haxx.se/windows/dl-7.68.0/curl-7.68.0-win64-mingw.zip
            URL_HASH SHA1=7eed952630d8b4ba42a157102249d1fd2d572e71
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${CMAKE_SOURCE_DIR}/deps/lib
            BUILD_IN_SOURCE 1
            BUILD_COMMAND ""
            UPDATE_COMMAND ${CMAKE_COMMAND} -E copy bin/curl-ca-bundle.crt ${CMAKE_SOURCE_DIR}/deps
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory include ${CMAKE_SOURCE_DIR}/deps/include
            )
else()
    ExternalProject_Add(curl
            GIT_REPOSITORY  https://github.com/curl/curl
            GIT_TAG fa009cc798f736e1751e62e47e4daee149cdc812
            BUILD_COMMAND ${CMAKE_COMMAND} .
            CMAKE_ARGS -DCURL_STATICLIB=ON -DSSL_ENABLED=ON -DCURL_DISABLE_LDAP=ON -DCURL_DISABLE_LDAPS=ON -DBUILD_CURL_EXE=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
            UPDATE_COMMAND ""
            )
endif()

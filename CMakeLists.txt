cmake_minimum_required(VERSION 3.0.2)
project(iota_simplewallet C)
set(CMAKE_C_STANDARD 99)
include(ExternalProject)

#Determine arch
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(x64 1)
    set(crypto_SUBDIR x64)
else()
    set(crypto_SUBDIR Win32)
endif()

### Main library
include_directories(${CMAKE_SOURCE_DIR}/deps/include)
link_directories(${CMAKE_SOURCE_DIR}/deps/lib)
link_directories(${CMAKE_SOURCE_DIR}/deps/bin)

option(USE_CURL_API "Use libcURL api to query nodes instead of iota.c" OFF)

#cURL based api queries
file(GLOB curl_src
        src/iota/curl/get_address_balance.c
        src/iota/curl/get_latest_inclusion.c
        src/iota/curl/get_transaction_inputs_to_address.c
        src/iota/curl/init.c
        )

#iota.c based api queries
file(GLOB iota_c_src
        src/iota/iota.c/get_address_balance.c
        src/iota/iota.c/get_latest_inclusion.c
        src/iota/iota.c/get_transaction_inputs_to_address.c
        src/iota/iota.c/init.c
        )

file(GLOB iota_simplewallet_SRC
        src/config/config.c
        src/config/http.c
        src/config/logger.c
        src/crypto/crypt.c
        src/crypto/password.c
        src/crypto/random.c
        src/database/sqlite3/db.c
        src/database/sqlite3/tables.c
        src/database/sqlite3/helpers/account.c
        src/database/sqlite3/stores/account.c
        src/database/sqlite3}stores/address.c
        src/database/sqlite3/stores/incoming_transaction.c
        src/database/sqlite3/stores/outgoing_transaction.c
        src/database/sqlite3/stores/input_to_output.c

        src/iota/common/core_client.c
        src/iota/common/generate_seed.c
        src/iota/common/generate_address.c

        src/iota/api.h

        deps/*.c
        )

if(USE_CURL_API)
    message("Using libcURL for IOTA network calls")
    add_library(iota_simplewallet
            ${iota_simplewallet_SRC}
            ${curl_src}
            )
    target_compile_definitions(iota_simplewallet PUBLIC "-DUSE_CURL_API")
else()
    add_library(iota_simplewallet
            ${iota_simplewallet_SRC}
            ${iota_c_src}
            )
endif()
unset(USE_CURL_API)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O3 -DFLEX_TRIT_ENCODING_3_TRITS_PER_BYTE -DCURL_STATICLIB")

target_link_libraries(iota_simplewallet libssl.a libssh2.a libnghttp2.a libcrypto.a libcurl.a libssl.a libcrypto.a libbrotlicommon-static.a libbrotlidec-static.a libbrotlienc-static.a libbrotlicommon-static.a libz.a wldap32 logger libsodium.a cclient common http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread ws2_32 wsock32 crypt32)
add_dependencies(iota_simplewallet curl crypto iota.c cjson)




#Should we build the cli app
set(BUILD_CLI "Build the cli application" ON)
if(BUILD_CLI)
    add_executable(wallet
            src/main.c
            )
    target_link_libraries(wallet iota_simplewallet crypto ssl libbrotlicommon-static.a libbrotlidec-static.a libbrotlienc-static.a libbrotlicommon-static.a wldap32)
endif(BUILD_CLI)


##
### Dependencies
##


#sqlite3

find_package (sqlite3)
if (SQLITE3_FOUND)
    message("Found Sqlite3")
    include_directories(${SQLITE3_INCLUDE_DIRS})
    target_link_libraries (iota_simplewallet ${SQLITE3_LIBRARIES})
else()
  ExternalProject_Add(sqlite3
          URL  https://sqlite.org/2020/sqlite-amalgamation-3310100.zip
          URL_HASH SHA1=a58e91a39b7b4ab720dbc843c201fb6a18eaf32b
          CONFIGURE_COMMAND ""
          BUILD_IN_SOURCE 1
          BUILD_ALWAYS 1
          BUILD_COMMAND ${CMAKE_COMMAND} -E copy ./sqlite3.h ${CMAKE_SOURCE_DIR}/deps/include/sqlite3.h
          INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ./sqlite3.c ${CMAKE_SOURCE_DIR}/deps/sqlite3.c

          )
endif (SQLITE3_FOUND)


#libsodium
if (WIN32)
    if(DEFINED x64)
        set(CRYPTO_FOLDER libsodium-win64)
    else()
        set(CRYPTO_FOLDER libsodium-win32)
    endif()
    set(SODIUM_STATIC 1)
    set(SODIUM_EXPORT)
    ExternalProject_Add(crypto
            URL "https://download.libsodium.org/libsodium/releases/libsodium-1.0.18-mingw.tar.gz"
            URL_HASH SHA1=9e7143e7f0e4232bd6b867cf37e7995165a220ae
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRYPTO_FOLDER}/include ${CMAKE_SOURCE_DIR}/deps/include
            BUILD_COMMAND ""
            BUILD_ALWAYS 1
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CRYPTO_FOLDER}/lib ${CMAKE_SOURCE_DIR}/deps/lib
            )
endif (WIN32)


#iota.c

#ugly hack for mingw
if (WIN32)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake-build-debug/iota.c-prefix/src/iota.c-build/mbedtls/src/ext_mbedtls-build/include/mbedtls")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake-build-debug/iota.c-prefix/src/iota.c-build/mbedtls/src/ext_mbedtls-build/scripts")
endif (WIN32)

#


ExternalProject_Add(iota.c
        GIT_REPOSITORY https://github.com/iotaledger/iota.c
        GIT_TAG 22b52d7c4333d6828ab48b811807be8081fbf15c
        UPDATE_COMMAND ""
        CMAKE_ARGS -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_LINK_LIBRARY_FLAG="-g -lws2_32 -lm" -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
        )

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


#cJSON
ExternalProject_Add(cjson
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON
        GIT_TAG 2955fe5ec49a672848a323a7c943c9525d16c83f
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.c ${CMAKE_SOURCE_DIR}/deps/
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.h ${CMAKE_SOURCE_DIR}/deps/
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ./cJSON.h ${CMAKE_SOURCE_DIR}/deps/include/cjson
        )

#iota.c ships with cJSON, but we are supplying our own version (issue with shipped cJSON lib on  win32)
add_dependencies(cjson iota.c)
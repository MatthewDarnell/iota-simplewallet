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

#deps we are building from source
add_library(deps
        deps/cJSON.c
        deps/sqlite3.c
        )

file(GLOB iota_simplewallet_SRC
        src/config/config.c
        src/crypto/crypt.c
        src/database/sqlite3/db.c
        src/database/sqlite3/tables.c
        src/database/sqlite3/stores/account.c
        src/database/sqlite3/stores/address.c
        src/database/sqlite3/stores/incoming_transaction.c
        src/database/sqlite3/stores/outgoing_transaction.c
        src/database/sqlite3/stores/input_to_output.c
        src/iota/test.c
        )

add_library(iota_simplewallet
    ${iota_simplewallet_SRC}
        )

target_link_libraries(iota_simplewallet deps logger libsodium.a cclient common http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread ws2_32 wsock32 )
add_dependencies(iota_simplewallet deps crypto iota.c cjson)


#Should we build the cli app
set(BUILD_CLI "Build the cli application" ON)
if(BUILD_CLI)
    add_executable(wallet
            src/main.c
            )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O0")
    target_link_libraries(wallet iota_simplewallet)
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
ExternalProject_Add(iota.c
        GIT_REPOSITORY https://github.com/iotaledger/iota.c
        GIT_TAG 22b52d7c4333d6828ab48b811807be8081fbf15c
        UPDATE_COMMAND ""
        CMAKE_ARGS -DCMAKE_EXE_LINKER_FLAGS="-fno-stack-protector" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_LINK_LIBRARY_FLAG="-g -lws2_32 -lm" -DCMAKE_CXX_FLAGS="-lm" -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
        )


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
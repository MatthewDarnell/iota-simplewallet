cmake_minimum_required(VERSION 3.11)
project(iota_simplewallet C)
set(CMAKE_C_STANDARD 99)


#Determine arch
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(x64 1)
    set(crypto_SUBDIR x64)
else()
    set(crypto_SUBDIR Win32)
endif()


include(ExternalProject)
include_directories(${CMAKE_SOURCE_DIR}/deps/include)
link_directories(${CMAKE_SOURCE_DIR}/deps/lib)
link_directories(${CMAKE_SOURCE_DIR}/deps/bin)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -Wall -O3 -DFLEX_TRIT_ENCODING_3_TRITS_PER_BYTE")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols -Wl,--whole-archive")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O3 -DFLEX_TRIT_ENCODING_3_TRITS_PER_BYTE")
endif()
include(${CMAKE_SOURCE_DIR}/src/cmake/iota.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/src/cmake/sqlite3.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/src/cmake/sodium.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/src/cmake/cjson.cmake)

### Main library
#option(BUILD_CLI "Build the cli application" ON)
set(BUILD_CLI ON)
option(USE_CURL_API "Use libcURL api to query nodes instead of iota.c" OFF)


#cURL based api queries
file(GLOB curl_src
        src/iota/curl/get_address_balance.c
        src/iota/curl/get_latest_inclusion.c
        src/iota/curl/get_transaction_inputs_to_address.c
        src/config/http.c
        src/iota/curl/init.c
        )

#iota.c based api queries
file(GLOB iota_c_src
        src/iota/iota.c/get_address_balance.c
        src/iota/iota.c/get_latest_inclusion.c
        src/iota/iota.c/get_transaction_inputs_to_address.c
        src/iota/iota.c/were_addresses_spent_from.c
        src/iota/iota.c/init.c
        )

file(GLOB iota_simplewallet_SRC
        src/api/account.c
        src/api/address.c
        src/api/transaction.c
        src/config/config.c
        src/config/logger.c
        src/crypto/crypt.c
        src/crypto/password.c
        src/crypto/random.c
        src/database/sqlite3/db.c
        src/database/sqlite3/tables.c
        src/database/helpers/generate_address.c
        src/database/helpers/get_inputs.c
        src/database/helpers/store_iota_inputs.c
        src/database/sqlite3/stores/account.c
        src/database/sqlite3/stores/address.c
        src/database/sqlite3/stores/incoming_transaction.c
        src/database/sqlite3/stores/outgoing_transaction.c
        src/iota/common/core_client.c
        src/iota/common/generate_address.c
        src/iota/common/generate_seed.c
        src/iota/common/get_inputs.c

        src/iota/iota.c/send_transaction.c
        src/iota/iota.c/send_trytes.c


        src/iota/api.h
        src/thread/address_balance_detector.c
        src/thread/deposit_detector.c
        src/thread/send_transaction.c
        src/thread/spent_from_detector.c
        src/thread/helper/start.c
        deps/sqlite3.c
        deps/cJSON.c
        )


if(USE_CURL_API)
    include(${CMAKE_CURRENT_LIST_DIR}/src/cmake/curl.cmake)
    message(STATUS "Using libcURL for IOTA network calls")
    add_library(iota_simplewallet SHARED
            ${iota_simplewallet_SRC}
            ${curl_src}
            )
    target_compile_definitions(iota_simplewallet PUBLIC "-DUSE_CURL_API")
    set(LIBRARIES "${LIBRARIES} libssl.a libssh2.a libnghttp2.a libcrypto.a libssl.a libcrypto.a libbrotlicommon-static.a libbrotlidec-static.a libbrotlienc-static.a libbrotlicommon-static.a libz.a libcurl.a wldap32 ")
    add_dependencies(iota_simplewallet curl openssl libssh2 nghttp2 brotli zlib)
    if(WIN32)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DCURL_STATICLIB")
    endif()
else()
    message(STATUS "Using iota.c for IOTA network calls")

    add_library(iota_simplewallet SHARED
            ${iota_simplewallet_SRC}
            ${iota_c_src}
            )
endif()



if(WIN32)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--whole-archive")
    set_target_properties(iota_simplewallet PROPERTIES
            LINK_FLAGS "-Wl,--whole-archive -Wl,--export-all-symbols"
            )
    if(USE_CURL_API)
      message("linking to curl")
      target_link_libraries(iota_simplewallet cclient libssl.a libssh2.a libnghttp2.a libcrypto.a libssl.a libcrypto.a libbrotlicommon-static.a libbrotlidec-static.a libbrotlienc-static.a libbrotlicommon-static.a libz.a libcurl.a wldap32 logger libsodium.a common http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread wldap32 ws2_32 wsock32 crypt32 libssl.a libssh2.a libnghttp2.a libcrypto.a libssl.a libcrypto.a libbrotlicommon-static.a libbrotlidec-static.a libbrotlienc-static.a libbrotlicommon-static.a libz.a libcurl.a wldap32)
    else()
        message("not linking to curl")
      target_link_libraries(iota_simplewallet cclient logger libsodium.a common http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread wldap32 ws2_32 wsock32 crypt32)
    endif()
else()
    target_link_libraries(iota_simplewallet cclient logger sodium common http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread m dl)
endif()

unset(USE_CURL_API)


add_dependencies(iota_simplewallet json libsodium iota.c-library sqlite)
install(TARGETS iota_simplewallet DESTINATION ${CMAKE_SOURCE_DIR}/deps/lib)


#Should we build the cli app
if(BUILD_CLI)
    include_directories(${CMAKE_SOURCE_DIR}/deps/include)
    file(GLOB wallet_src
            src/wallet/cli/cli.c
            src/wallet/main.c
            )


    add_executable(wallet
            ${wallet_src}
            )
    link_directories(${CMAKE_BINARY_DIR})
    target_link_libraries(wallet iota_simplewallet)
endif(BUILD_CLI)

add_dependencies(json iota.c-library)









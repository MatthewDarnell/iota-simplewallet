cmake_minimum_required(VERSION 3.11)
project(iota_simplewallet C)
set(CMAKE_C_STANDARD 99)

set(IOTA_SIMPLEWALLET_VERSION_MAJOR 0 CACHE STRING "major version of memory" FORCE)
set(IOTA_SIMPLEWALLET_VERSION_MINOR 1 CACHE STRING "minor version of memory" FORCE)
set(IOTA_SIMPLEWALLET_VERSION_PATCH 0 CACHE STRING "patch version of memory" FORCE)
set(IOTA_SIMPLEWALLET_VERSION "${IOTA_SIMPLEWALLET_VERSION_MAJOR}.${IOTA_SIMPLEWALLET_VERSION_MINOR}.${IOTA_SIMPLEWALLET_VERSION_PATCH}"
                             CACHE STRING "version of memory" FORCE)

include(GNUInstallDirs)

#Determine arch
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(x64 1)
    set(crypto_SUBDIR x64)
else()
    set(crypto_SUBDIR Win32)
endif()

set(DEPS_DIR "${CMAKE_BINARY_DIR}/deps")

include(ExternalProject)
include(FetchContent)
include_directories(${DEPS_DIR}/include)
include_directories(${DEPS_DIR}/include/keccak)
link_directories(${DEPS_DIR}/lib)
link_directories(${DEPS_DIR}/bin)


if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -Wall -O3 -DFLEX_TRIT_ENCODING_3_TRITS_PER_BYTE -DSQLITE_THREADSAFE=1")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols -Wl,--whole-archive")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O3 -DFLEX_TRIT_ENCODING_3_TRITS_PER_BYTE -DSQLITE_THREADSAFE=1")
endif()
include(${CMAKE_CURRENT_LIST_DIR}/cmake/iota.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/sqlite3.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/sodium.cmake)

### Main library
#option(BUILD_CLI "Build the cli application" ON)
set(BUILD_CLI ON)

#iota.c based api queries
file(GLOB iota_c_src
        src/iota/iota.c/get_address_balance.c
        src/iota/iota.c/get_latest_inclusion.c
        src/iota/iota.c/get_node_info.c
        src/iota/iota.c/get_transaction_inputs_to_address.c
        src/iota/iota.c/were_addresses_spent_from.c
        src/iota/iota.c/init.c
        )

file(GLOB iota_simplewallet_SRC
        src/api/account.c
        src/api/address.c
        src/api/event.c
        src/api/info.c
        src/api/init.c
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
        src/database/sqlite3/stores/user_data.c

        src/event/event_store.c

        src/iota/common/core_client.c
        src/iota/common/generate_address.c
        src/iota/common/generate_seed.c
        src/iota/common/get_inputs.c

        src/iota/iota.c/send_transaction.c
        src/iota/iota.c/send_trytes.c

        src/iota/api.h

        src/thread/address_balance_detector.c
        src/thread/deposit_detector.c
        src/thread/event_queue.c
        src/thread/node_info_updater.c
        src/thread/send_transaction.c
        src/thread/sent_transaction_confirmer.c
        src/thread/spent_from_detector.c

        src/thread/helper/start.c

        ${DEPS_DIR}/sqlite3.c
        )


add_library(iota_simplewallet SHARED
        ${iota_simplewallet_SRC}
        ${iota_c_src}
        )
target_include_directories(iota_simplewallet PRIVATE ${DEPS_DIR}/include/cjson)

add_dependencies(iota_simplewallet iota.c-library libsodium)

if(WIN32)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--whole-archive")
    set_target_properties(iota_simplewallet PROPERTIES
            LINK_FLAGS "-Wl,--whole-archive -Wl,--export-all-symbols"
            )
    target_link_libraries(iota_simplewallet PRIVATE cclient logger libsodium.a common cjson http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread wldap32 ws2_32 wsock32 crypt32)
else()
    target_link_libraries(iota_simplewallet PRIVATE cclient logger sodium common cjson http_parser mbedtls mbedx509 mbedcrypto unity keccak pthread m dl)
endif()

set(CMAKE_IOTA_CONFIG_INSTALL_DIR "lib/cmake/iota_simplewallet")

target_include_directories(iota_simplewallet PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS iota_simplewallet
	EXPORT iota_simplewalletTargets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})


install(FILES "${CMAKE_SOURCE_DIR}/src/iota-simplewallet.h"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/iota)

set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"cmake/${PROJECT_NAME}Config.cmake.in"
	"${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

write_basic_package_version_file(
	"${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${IOTA_SIMPLEWALLET_VERSION}
	COMPATIBILITY SameMajorVersion)

install(
	EXPORT iota_simplewalletTargets
	NAMESPACE iota::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(
	FILES
  		"${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
  		"${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")


# Write/install version file
include(CMakePackageConfigHelpers)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/cmake/iota-simplewallet-config-version.cmake")
write_basic_package_version_file(${version_file}
                                 VERSION ${IOTA_SIMPLEWALLET_VERSION}
                                 COMPATIBILITY AnyNewerVersion)

#Should we build the cli app
if(BUILD_CLI)
    include_directories(${DEPS_DIR}/include)
    file(GLOB wallet_src
            src/wallet/cli/cli.c
            src/wallet/main.c
            )


    add_executable(wallet
            ${wallet_src}
            )
    link_directories(${CMAKE_BINARY_DIR})
    target_link_libraries(wallet iota_simplewallet pthread)
endif(BUILD_CLI)
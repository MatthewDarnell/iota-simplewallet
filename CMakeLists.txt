cmake_minimum_required(VERSION 3.0.2)
project(iota_simplewallet C)
set(CMAKE_C_STANDARD 99)
include(ExternalProject)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(x64 1)
    set(crypto_SUBDIR x64)
else()
    set(crypto_SUBDIR Win32)
endif()



### Main app
include_directories(${CMAKE_SOURCE_DIR}/deps/include)
link_directories(${CMAKE_SOURCE_DIR}/deps/lib)


add_executable(iota_simplewallet main.c)
target_link_libraries(iota_simplewallet cclient cjson common http_parser libsodium sqlite3 pthread)


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
  if (Win32)
      if(DEFINED x64)
        set(SQLITE_URL https://www.sqlite.org/2020/sqlite-dll-win64-x64-3310100.zip)
        set(SQLITE_HASH 300c5f26feb297968f06790c5b1e19db9347da67)
      else()
          set(SQLITE_URL https://www.sqlite.org/2020/sqlite-dll-win32-x86-3310100.zip)
          set(SQLITE_HASH 3475dccc0378a0b2407ae78725d1a18d7885cdd5)
      endif()
  endif(Win32)

  ExternalProject_Add(sqlite3
          URL ${SQLITE_URL}
          URL_HASH SHA1=${SQLITE_HASH}
          CONFIGURE_COMMAND ""
          BUILD_IN_SOURCE 1
          BUILD_COMMAND ""
          BUILD_ALWAYS 1
          INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ./ ${CMAKE_SOURCE_DIR}/deps/lib
          )
endif (SQLITE3_FOUND)


#libsodium
if (WIN32)
    ExternalProject_Add(crypto
            URL "https://download.libsodium.org/libsodium/releases/libsodium-1.0.18-stable-msvc.zip"
            URL_HASH SHA1=bc6ddb54c4e8278c90d4bea26966edd2ceb03d15
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory include ${CMAKE_SOURCE_DIR}/deps/include
            BUILD_COMMAND ""
            BUILD_ALWAYS 1
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${crypto_SUBDIR}/Release/v142/static ${CMAKE_SOURCE_DIR}/deps/lib
            )
    set(SODIUM_STATIC 1)
    set(SODIUM_EXPORT)

endif (WIN32)


#iota.c
if (WIN32)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake-build-debug/iota.c-prefix/src/iota.c-build/mbedtls/src/ext_mbedtls-build/include/mbedtls")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake-build-debug/iota.c-prefix/src/iota.c-build/mbedtls/src/ext_mbedtls-build/scripts")
endif (WIN32)

ExternalProject_Add(iota.c
        GIT_REPOSITORY https://github.com/iotaledger/iota.c
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/deps
        )


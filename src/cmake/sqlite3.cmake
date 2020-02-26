
#sqlite3
set(SQLITE3_PATH "${CMAKE_CURRENT_BINARY_DIR}/sqlite-prefix/src/sqlite")
ExternalProject_Add(sqlite
        URL  https://sqlite.org/2020/sqlite-amalgamation-3310100.zip
        URL_HASH SHA1=a58e91a39b7b4ab720dbc843c201fb6a18eaf32b
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS 1
        CONFIGURE_COMMAND ${CMAKE_C_COMPILER} -c ${SQLITE3_PATH}/sqlite3.c
        BUILD_COMMAND ${CMAKE_AR} rcs ${CMAKE_SOURCE_DIR}/deps/lib/libsqlite3.a ${SQLITE3_PATH}/sqlite3.o
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ./sqlite3.h ${CMAKE_SOURCE_DIR}/deps/include/sqlite3.h
        )


#sqlite3
ExternalProject_Add(sqlite
        URL  https://sqlite.org/2020/sqlite-amalgamation-3310100.zip
        URL_HASH SHA1=a58e91a39b7b4ab720dbc843c201fb6a18eaf32b
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS 1
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ./sqlite3.c ${CMAKE_SOURCE_DIR}/deps/sqlite3.c
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ./sqlite3.h ${CMAKE_SOURCE_DIR}/deps/include/sqlite3.h
        )

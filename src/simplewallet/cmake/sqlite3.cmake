
#sqlite3

FetchContent_Declare(
  sqlite
  URL  https://sqlite.org/2020/sqlite-amalgamation-3310100.zip
  URL_HASH SHA1=a58e91a39b7b4ab720dbc843c201fb6a18eaf32b
)	

FetchContent_GetProperties(sqlite)
if(NOT sqlite_POPULATED)
  FetchContent_Populate(sqlite)
  file(COPY ${sqlite_SOURCE_DIR}/sqlite3.c DESTINATION ${DEPS_DIR})
  file(COPY ${sqlite_SOURCE_DIR}/sqlite3.h DESTINATION ${DEPS_DIR}/include)
endif()
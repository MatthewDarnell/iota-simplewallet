
#cJSON


FetchContent_Declare(
  cjson
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON
  GIT_TAG 2955fe5ec49a672848a323a7c943c9525d16c83f
)       

FetchContent_GetProperties(cjson)
if(NOT cjson_POPULATED)
  FetchContent_Populate(cjson)
  file(COPY ${cjson_SOURCE_DIR}/cJSON.c DESTINATION ${DEPS_DIR})
  file(COPY ${cjson_SOURCE_DIR}/cJSON.h DESTINATION ${DEPS_DIR}/include/cjson)
endif()

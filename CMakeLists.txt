cmake_minimum_required(VERSION 3.15.6)
project(iota-qt-wallet VERSION 0.0.1.0 LANGUAGES CXX)

include(ExternalProject)

set(SIMPLEWALLET_INSTALL_DIR "${CMAKE_BINARY_DIR}/iota_simplewallet/output")
set(SIMPLEWALLET_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/simplewallet")
set(GUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/gui")

ExternalProject_Add(iota_simplewallet
  PREFIX iota_simplewallet
  SOURCE_DIR "${SIMPLEWALLET_SOURCE_DIR}"
  BUILD_ALWAYS 1
  CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=${SIMPLEWALLET_INSTALL_DIR}
)

ExternalProject_Add(iota-qt
  PREFIX iota-qt
  SOURCE_DIR "${GUI_SOURCE_DIR}"
  BUILD_ALWAYS 1
  CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
        -Diota_simplewallet_DIR:PATH=${SIMPLEWALLET_INSTALL_DIR}/lib/cmake/iota_simplewallet
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
  DEPENDS iota_simplewallet
)

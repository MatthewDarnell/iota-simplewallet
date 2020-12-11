cmake_minimum_required(VERSION 3.15.6)
project(iota-qt VERSION 0.0.1.0 LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
    
    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD 14)

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    if(WIN32)
        add_definitions(-DWIN32 -D_UNICODE -DUNICODE -D_WIN32_WINNT=0x0601 -DNOMINMAX)
    endif()

    set(CMAKE_INSTALL_ALWAYS TRUE)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    #include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(STATUS "Doxygen not found, not building docs")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

find_package(Qt5 5.9 COMPONENTS
    Core
    Network
    Concurrent
    Widgets
    REQUIRED)

include(selectdeployqt)

## use, i.e. don't skip the full RPATH for the build tree
#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

## when building, don't use the install RPATH already
## (but later on when installing)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

#set(CMAKE_INSTALL_RPATH "${QT_LIBRARY_DIR}")

## add the automatically determined parts of the RPATH
## which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# libraries
find_package(iota_simplewallet REQUIRED)

# executables
add_subdirectory(src/core)
add_subdirectory(src/qt)

set(CPACK_GENERATOR "TBZ2")
set(CPACK_PACKAGE_NAME "iota-qt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An Iota-Qt wallet")
set(CPACK_PACKAGE_VERSION_MAJOR "${iota-qt_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${iota-qt_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${iota-qt_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${iota-qt_VERSION}")

#if(WIN32)

#endif()
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_VOLUME_NAME "Iota-Qt")
    set(CPACK_SYSTEM_NAME "OSX")
    set(CPACK_PACKAGE_FILE_NAME "Iota-Qt-${iota-qt_VERSION}")
endif()
#elseif(UNIX)
#  set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
#endif()

include(CPack)

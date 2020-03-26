# get revision
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

file(GLOB SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp,*.h)
file(GLOB UI_FILES forms/*.ui)

if(APPLE)
    file(GLOB OBJC_SOURCES *.mm)
#    set_source_files_properties(${OBJC_SOURCES} PROPERTIES
#                                COMPILE_FLAGS "-x objective-c++")
    list(APPEND SOURCES ${OBJC_SOURCES})
endif()

add_custom_target(forms SOURCES ${UI_FILES})

set(QRC_FILES "bitcoin.qrc")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/iotaqtconfig.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/iotaqtconfig.cpp" @ONLY)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/iotaqtconfig.cpp")

if(CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    find_package(Qt5QuickCompiler REQUIRED)
    qtquick_compiler_add_resources(RESOURCES ${QRC_FILES})
else()
    qt5_add_resources(RESOURCES ${QRC_FILES})
endif()

#if(WIN32)
#    list(APPEND RESOURCES assets/winResourse.rc)
#    set(GUI_TYPE WIN32)
#endif(WIN32)

## Set some Apple MacOS Specific settings

set(EXTRA_LIBS "")

if(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
    find_library(ObjC objc REQUIRED)
    find_library(CoreFoundation CoreFoundation REQUIRED)
    find_library(IOKit IOKit REQUIRED)
    find_library(AppKit AppKit REQUIRED)
    set(EXTRA_LIBS ${ObjC} ${CoreFoundation} ${IOKit} ${AppKit})
endif()

add_executable(iota-qt ${GUI_TYPE} ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(iota-qt
    core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Concurrent
    ${EXTRA_LIBS})


target_compile_options(iota-qt PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4>)

set_target_properties(iota-qt
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/res/Info.plist"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/appdir/usr/bin"
)

install(TARGETS iota-qt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION appdir/usr/bin
)

if(APPLE)
    set(APP_EXE "$<TARGET_FILE_NAME:iota-qt>.app")
else()
    set(APP_EXE "$<TARGET_FILE_NAME:iota-qt>")
endif()

install(CODE "execute_process(COMMAND ${CMAKE_COMMAND}
    -DQMAKE=${QT_QMAKE_EXECUTABLE}
    -DAPP_EXE=\${CMAKE_INSTALL_PREFIX}/${APP_EXE}
    -DDEPLOY_TOOL=${DEPLOY_TOOL}
    -P ${CMAKE_SOURCE_DIR}/cmake/platformdeployqt.cmake)")

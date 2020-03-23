if(NOT DEFINED Qt5_DIR)
    message(FATAL_ERROR "Qt5_DIR is not defined")
endif()
if(NOT QT_BINARY_DIR)
    if(NOT QT_QMAKE_EXECUTABLE)
        find_program(QT_QMAKE_EXECUTABLE_FINDQT NAMES qmake qmake5 qmake-qt5
            PATHS "${Qt5_DIR}/../../../bin")
        set(QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE_FINDQT} CACHE PATH "Qt qmake program.")
    endif(NOT QT_QMAKE_EXECUTABLE)

    exec_program(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_BINS" OUTPUT_VARIABLE QTBINS)
    set(QT_BINARY_DIR ${QTBINS} CACHE INTERNAL "" FORCE)
endif(NOT QT_BINARY_DIR)

if(WIN32)
    find_program(_DEPLOY_TOOL NAMES windeployqt PATHS "${QT_BINARY_DIR}")
elseif(APPLE)
    find_program(_DEPLOY_TOOL NAMES macdeployqt PATHS "${QT_BINARY_DIR}")
else()
    set(DEPLOY_DIR "${CMAKE_SOURCE_DIR}/../deploy")
    find_program(_DEPLOY_TOOL NAMES linuxdeployqt linuxdeployqt-continuous-x86_64.AppImage PATHS "${DEPLOY_DIR}/tools/bin")
endif()

set(DEPLOY_TOOL ${_DEPLOY_TOOL} CACHE PATH "Qt deploy tool")

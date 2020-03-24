if(NOT DEFINED DEPLOY_TOOL)
    message(FATAL_ERROR "No deploy tool specified, use -DDEPLOY_TOOL")
endif()

if(NOT DEFINED QMAKE)
    message(FATAL_ERROR "No qmake specified, use -DQMAKE")
endif()

if(NOT DEFINED APP_EXE)
    message(FATAL_ERROR "No executable specified, use -DAPP_EXE")
endif()

if(WIN32)
    if(NOT DEFINED QML_DIR)
        execute_process(COMMAND ${DEPLOY_TOOL} --release "${APP_EXE}")
    else()
        execute_process(COMMAND ${DEPLOY_TOOL} --release "${APP_EXE}")
    endif()
elseif(APPLE)
    execute_process(COMMAND ${DEPLOY_TOOL} ${APP_EXE})
else()
    set(EXCLUDE_LIST "libnspr4.so,libnss3.so,libnssutil3.so,libsmime3")
    execute_process(COMMAND ${DEPLOY_TOOL} ${APP_EXE} -qmake=${QMAKE} -appimage -unsupported-allow-new-glibc -exclude-libs=${EXCLUDE_LIST})
endif()
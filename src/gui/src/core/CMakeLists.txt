#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")

#find_package(Qt5 REQUIRED Core)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp,*.h)

add_library(core STATIC ${SOURCES} ${HEADERS})

# We need this directory, and users of our library will need it too
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(core
    iota::iota_simplewallet
    Qt5::Core)

target_compile_options(core PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)

if(WIN32)
    set(RUNTIME_DESTINATION appdir)
else()
    set(RUNTIME_DESTINATION appdir/usr/bin)
endif()


if(WIN32)
	get_target_property(DLL_LOCATION iota::iota_simplewallet IMPORTED_LOCATION_RELEASE)
	install(FILES ${DLL_LOCATION} DESTINATION ${RUNTIME_DESTINATION})
endif()
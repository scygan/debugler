set(dglcommon_SOURCES 
    gl-types.cpp
	gl-entrypoints.cpp
	os.cpp ipc.cpp
	version.cpp
    )

include_directories(
    ..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    )

add_library(dglcommon
    ${dglcommon_SOURCES}
)
add_dependencies(dglcommon codegen)

set_property(TARGET dglcommon PROPERTY COMPILE_FLAGS "-fPIC")

set(dglnet_SOURCES 
    client.cpp server.cpp transport.cpp
	protocol/message.cpp protocol/entrypoint.cpp protocol/resource.cpp
	protocol/pixeltransfer.cpp protocol/ctxobjname.cpp
	protocol/request.cpp
    )

add_library(dglnet
    ${dglnet_SOURCES}
)

target_link_libraries(dglnet dglcommon)

set_property(TARGET dglnet PROPERTY COMPILE_FLAGS "-fPIC")


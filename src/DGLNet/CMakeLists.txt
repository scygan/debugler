set(dglnet_SOURCES 
    client.cpp server.cpp transport.cpp
	protocol/message.cpp protocol/entrypoint.cpp protocol/resource.cpp
	protocol/pixeltransfer.cpp
    )

add_library(dglnet
    ${dglnet_SOURCES}
)
set_property(TARGET dglnet PROPERTY COMPILE_FLAGS "-fPIC")


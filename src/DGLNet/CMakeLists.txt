set(dglnet_SOURCES 
    client.cpp message.cpp
    server.cpp transport.cpp
    )

add_library(dglnet
    ${dglnet_SOURCES}
)
set_property(TARGET dglnet PROPERTY COMPILE_FLAGS "-fPIC")


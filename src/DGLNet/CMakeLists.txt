set(dglnet_SOURCES 
    client.cpp message.cpp
    server.cpp transport.cpp
    )

add_library(dglnet
    ${dglnet_SOURCES}
)


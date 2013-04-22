set(dglloader_SOURCES 
    main.cpp
    )


add_executable(dglloader
    ${dglloader_SOURCES}
)

add_dependencies(dglloader Boost)

target_link_libraries(dglloader boost_program_options dglcommon rt)

install (TARGETS dglloader DESTINATION lib)

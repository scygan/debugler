set(dglloader_SOURCES 
    main.cpp process.cpp
    )


add_executable(dglloader
    ${dglloader_SOURCES}
)

add_dependencies(dglloader dglinject)

if (ANDROID)
	set(libs log)
else()
	set(libs rt pthread)
endif()
target_link_libraries(dglloader dglinject boost_program_options dglcommon ${libs})

install (TARGETS dglloader DESTINATION bin)

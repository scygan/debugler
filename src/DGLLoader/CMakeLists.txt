set(dglloader_SOURCES 
    main.cpp process.cpp
    )


add_executable(dglloader
    ${dglloader_SOURCES}
)

add_dependencies(dglloader Boost)

if (ANDROID)
	set(libs log)
else()
	set(libs rt)
endif()
target_link_libraries(dglloader boost_program_options dglcommon ${libs})

install (TARGETS dglloader DESTINATION bin)

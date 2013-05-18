set(dglloader_SOURCES 
    main.cpp
    )


add_executable(dglloader
    ${dglloader_SOURCES}
)

add_dependencies(dglloader Boost)

if (ANDROID)
	set(librt "")
else()
	set(librt rt)
endif()
target_link_libraries(dglloader boost_program_options dglcommon ${librt})

install (TARGETS dglloader DESTINATION bin)

if(ARMEABI)
	set(OBJCOPY_MACHINE "arm")
	set(OBJCOPY_FORMAT  "elf32-littlearm")
elseif (X86_64) 
	set(OBJCOPY_MACHINE "i386")
	set(OBJCOPY_FORMAT  "elf64-x86-64")
	#64-bit version of installer should contain also 32-bit files
	set(COMPAT_32 android-x86)
	set(COMPAT_32_BIN_PATH "../../../../${COMPAT_32}/${CMAKE_BUILD_TYPE}/bin/x86/")
	set(COMPAT_32_LIBS_PATH "../../../../${COMPAT_32}/${CMAKE_BUILD_TYPE}/libs/x86/")
elseif (X86) 
	set(OBJCOPY_MACHINE "i386")
	set(OBJCOPY_FORMAT  "elf32-i386")
elseif (MIPS)
	set(OBJCOPY_MACHINE "mips:isa32")
	set(OBJCOPY_FORMAT  "elf32-tradlittlemips")
endif()


add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/libdglwrapper.stripped.so
    COMMAND ${CMAKE_STRIP} ${LIBRARY_OUTPUT_PATH}/libdglwrapper.so --strip-unneeded -o ${LIBRARY_OUTPUT_PATH}/libdglwrapper.stripped.so
    DEPENDS dglwrapper
    COMMENT "stripping libdglwrapper"
	WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
    VERBATIM
    )
 
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/libdglwrapper_pack.o
    COMMAND ${CMAKE_OBJCOPY} -I binary -B ${OBJCOPY_MACHINE} -O ${OBJCOPY_FORMAT} libdglwrapper.stripped.so libdglwrapper_pack.o
    DEPENDS ${LIBRARY_OUTPUT_PATH}/libdglwrapper.stripped.so
    COMMENT "Packing libdglwrapper"
	WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
    VERBATIM
    )
    
add_custom_command(
    OUTPUT ${EXECUTABLE_OUTPUT_PATH}/dglloader.stripped
    COMMAND ${CMAKE_STRIP}  ${EXECUTABLE_OUTPUT_PATH}/dglloader --strip-unneeded -o ${EXECUTABLE_OUTPUT_PATH}/dglloader.stripped
    DEPENDS dglloader
    COMMENT "stripping dglloader"
	WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
    VERBATIM
    )
	
add_custom_command(
    OUTPUT ${EXECUTABLE_OUTPUT_PATH}/dglloader_pack.o
    COMMAND ${CMAKE_OBJCOPY} -I binary  -B ${OBJCOPY_MACHINE} -O ${OBJCOPY_FORMAT} dglloader.stripped dglloader_pack.o
    DEPENDS ${EXECUTABLE_OUTPUT_PATH}/dglloader.stripped
    COMMENT "Packing dglloader"
	WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    VERBATIM
    )
	
	
if (COMPAT_32) 
	add_definitions(-DCOMPAT_32)
	add_custom_command(
		OUTPUT ${LIBRARY_OUTPUT_PATH}/libdglwrapper-compat32.stripped.so
		COMMAND ${CMAKE_STRIP} ${LIBRARY_OUTPUT_PATH}/${COMPAT_32_LIBS_PATH}libdglwrapper.so --strip-unneeded -o ${LIBRARY_OUTPUT_PATH}/libdglwrapper-compat32.stripped.so
		COMMENT "stripping libdglwrapper-compat32"
		WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
		VERBATIM
		)
	 
	add_custom_command(
		OUTPUT ${LIBRARY_OUTPUT_PATH}/libdglwrapper_pack-compat32.o
		COMMAND ${CMAKE_OBJCOPY} -I binary -B ${OBJCOPY_MACHINE} -O ${OBJCOPY_FORMAT} libdglwrapper-compat32.stripped.so libdglwrapper_pack-compat32.o
		DEPENDS ${LIBRARY_OUTPUT_PATH}/libdglwrapper-compat32.stripped.so
		COMMENT "Packing libdglwrapper-compat32"
		WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
		VERBATIM
		)
		
	add_custom_command(
		OUTPUT ${EXECUTABLE_OUTPUT_PATH}/dglloader-compat32.stripped
		COMMAND ${CMAKE_STRIP}  ${EXECUTABLE_OUTPUT_PATH}/${COMPAT_32_BIN_PATH}/dglloader --strip-unneeded -o ${EXECUTABLE_OUTPUT_PATH}/dglloader-compat32.stripped
		COMMENT "stripping dglloader-compat32"
		WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
		VERBATIM
		)
		
	add_custom_command(
		OUTPUT ${EXECUTABLE_OUTPUT_PATH}/dglloader_pack-compat32.o
		COMMAND ${CMAKE_OBJCOPY} -I binary  -B ${OBJCOPY_MACHINE} -O ${OBJCOPY_FORMAT} dglloader-compat32.stripped dglloader_pack-compat32.o
		DEPENDS ${EXECUTABLE_OUTPUT_PATH}/dglloader-compat32.stripped
		COMMENT "Packing dglloader"
		WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
		VERBATIM
		)
		
	set (COMPAT_32_SOURCES
		${LIBRARY_OUTPUT_PATH}/libdglwrapper_pack-compat32.o
		${EXECUTABLE_OUTPUT_PATH}/dglloader_pack-compat32.o
		)
endif()
	
set(android-installer_SOURCES 
	${LIBRARY_OUTPUT_PATH}/libdglwrapper_pack.o
	${EXECUTABLE_OUTPUT_PATH}/dglloader_pack.o
	${COMPAT_32_SOURCES}
    installer.cpp
    )

add_executable(dglandroidinstaller
    ${android-installer_SOURCES}
)

target_link_libraries(dglandroidinstaller dglcommon)
	

if (ANDROID_PREBUILD)
    install(FILES ${CMAKE_BINARY_DIR}/../../android-x86/${CMAKE_BUILD_TYPE}/bin/x86/dglandroidinstaller     DESTINATION share/${CMAKE_PROJECT_NAME}/android-x86/)
	install(FILES ${CMAKE_BINARY_DIR}/../../android-x86_64/${CMAKE_BUILD_TYPE}/bin/x86_64/dglandroidinstaller     DESTINATION share/${CMAKE_PROJECT_NAME}/android-x86_64/)
    install(FILES ${CMAKE_BINARY_DIR}/../../android-arm/${CMAKE_BUILD_TYPE}/bin/armeabi/dglandroidinstaller DESTINATION share/${CMAKE_PROJECT_NAME}/android-arm/)
    install(FILES ${CMAKE_BINARY_DIR}/../../android-mips/${CMAKE_BUILD_TYPE}/bin/mips/dglandroidinstaller   DESTINATION share/${CMAKE_PROJECT_NAME}/android-mips/)
endif()

set(CPACK_GENERATOR "DEB")

if (ANDROID_PREBUILD)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-withAndroid)
endif()

string(REPLACE "\"" "" DGL_VERSION_TMP ${DGL_VERSION})
string(REPLACE "." ";" DGL_VERSION_STRING ${DGL_VERSION_TMP})

list(GET DGL_VERSION_STRING 0 MAJOR)
list(GET DGL_VERSION_STRING 1 MINOR)
list(GET DGL_VERSION_STRING 2 PATCH)

set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PATCH})

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtgui4,libqtcore4,xdg-utils")
set(CPACK_PACKAGE_DESCRIPTION "The OpenGL Debugger")
set(CPACK_PACKAGE_CONTACT "Slawomir Cygan debugler@gmail.com")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst;${CMAKE_CURRENT_SOURCE_DIR}/Debian/postrm;")

include(CPack)

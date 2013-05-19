find_program(XDG-MIME_EXECUTABLE xdg-mime)
find_program(XDG-DESKTOP-MENU_EXECUTABLE xdg-desktop-menu)
find_program(XDG-ICON-RESOURCE_EXECUTABLE xdg-icon-resource)

set (icons_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../DGLGui/icons/)

INSTALL (CODE "
execute_process(COMMAND ${XDG-MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/debugler-mime.xml)
execute_process(COMMAND ${XDG-DESKTOP-MENU_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/debugler.desktop)
execute_process(COMMAND ${XDG-MIME_EXECUTABLE} default debugler.desktop application/x-debugler)
execute_process(COMMAND ${XDG-ICON-RESOURCE_EXECUTABLE} install --novendor --size 256 ${icons_dir}/debugler-0.png debugler)
execute_process(COMMAND ${XDG-ICON-RESOURCE_EXECUTABLE} install --novendor --size 48  ${icons_dir}/debugler-1.png debugler)
execute_process(COMMAND ${XDG-ICON-RESOURCE_EXECUTABLE} install --novendor --size 24  ${icons_dir}/debugler-2.png debugler)
")

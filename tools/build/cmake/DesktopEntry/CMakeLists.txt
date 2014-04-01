set (icons_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/DGLGui/icons)

install (FILES debugler-mime.xml DESTINATION /tmp/debugler-inst/)
install (FILES debugler.desktop DESTINATION /tmp/debugler-inst/)
install (FILES ${icons_dir}/debugler-0.png DESTINATION /tmp/debugler-inst/)
install (FILES ${icons_dir}/debugler-1.png DESTINATION /tmp/debugler-inst/)
install (FILES ${icons_dir}/debugler-2.png DESTINATION /tmp/debugler-inst/)


find_program(XDG-MIME_EXECUTABLE xdg-mime)
find_program(XDG-DESKTOP-MENU_EXECUTABLE xdg-desktop-menu)
find_program(XDG-ICON-RESOURCE_EXECUTABLE xdg-icon-resource)



install (CODE "

function(do_run what)
    message(\"Running \${what}...\")
    execute_process(COMMAND \${what})
endfunction(do_run)

do_run(\"${XDG-MIME_EXECUTABLE} install --mode system --novendor \$ENV{DESTDIR}/tmp/debugler-inst/debugler-mime.xml\")
do_run(\"${XDG-DESKTOP-MENU_EXECUTABLE} install --mode system --novendor \$ENV{DESTDIR}/tmp/debugler-inst/debugler.desktop\")
do_run(\"${XDG-ICON-RESOURCE_EXECUTABLE} install --mode system --novendor --size 256 \$ENV{DESTDIR}/tmp/debugler-inst/debugler-0.png debugler\")
do_run(\"${XDG-ICON-RESOURCE_EXECUTABLE} install --mode system --novendor --size 48  \$ENV{DESTDIR}/tmp/debugler-inst/debugler-1.png debugler\")
do_run(\"${XDG-ICON-RESOURCE_EXECUTABLE} install --mode system --novendor --size 24  \$ENV{DESTDIR}/tmp/debugler-inst/debugler-2.png debugler\")
do_run(\"rm -r \$ENV{DESTDIR}/tmp/debugler-inst/\")
")

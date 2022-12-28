install(FILES ${CMAKE_SOURCE_DIR}/app/data/org.graceful.Terminator.desktop DESTINATION /usr/share/applications/)
install(FILES ${CMAKE_SOURCE_DIR}/app/data/org.graceful.Terminator.metainfo.xml DESTINATION /usr/share/metainfo/)
install(FILES ${CMAKE_SOURCE_DIR}/app/data/org.graceful.Terminator.service DESTINATION /usr/share/dbus-1/services/)
install(FILES ${CMAKE_SOURCE_DIR}/app/data/org.graceful.Terminator.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(FILES ${CMAKE_SOURCE_DIR}/app/data/org.graceful.Terminator-symbolic.svg DESTINATION /usr/share/icons/hicolor/symbolic/apps/)

install(CODE "execute_process(COMMAND glib-compile-schemas /usr/share/glib-2.0/schemas/)")
install(CODE "execute_process(COMMAND update-desktop-database -q /usr/share/applications/)")
install(CODE "execute_process(COMMAND gtk4-update-icon-cache -q -t -f /usr/share/icons/hicolor/)")

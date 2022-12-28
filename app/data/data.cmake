install(FILES ${CMAKE_SOURCE_DIR}/app/data/org.graceful.Terminator.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(CODE "execute_process(COMMAND glib-compile-schemas /usr/share/glib-2.0/schemas/)")

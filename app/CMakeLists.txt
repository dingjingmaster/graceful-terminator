
FILE(GLOB GT_SRC
        ${CMAKE_SOURCE_DIR}/app/gt-log.h

        ${CMAKE_SOURCE_DIR}/app/gt-enums.h
        ${CMAKE_SOURCE_DIR}/app/gt-enums.c

        ${CMAKE_SOURCE_DIR}/app/gt-vte-util.h
        ${CMAKE_SOURCE_DIR}/app/gt-vte-util.c

        ${CMAKE_SOURCE_DIR}/app/gt-application.h
        ${CMAKE_SOURCE_DIR}/app/gt-application.c

        ${CMAKE_SOURCE_DIR}/app/gt-close-dialog.h
        ${CMAKE_SOURCE_DIR}/app/gt-close-dialog.c
        ${CMAKE_SOURCE_DIR}/app/gt-close-dialog.ui

        ${CMAKE_SOURCE_DIR}/app/gt-settings.h
        ${CMAKE_SOURCE_DIR}/app/gt-settings.c

        ${CMAKE_SOURCE_DIR}/app/gt-tab.h
        ${CMAKE_SOURCE_DIR}/app/gt-tab.c
        ${CMAKE_SOURCE_DIR}/app/gt-tab.ui

        ${CMAKE_SOURCE_DIR}/app/gt-pages.c
        ${CMAKE_SOURCE_DIR}/app/gt-pages.h
        ${CMAKE_SOURCE_DIR}/app/gt-pages.ui

        ${CMAKE_SOURCE_DIR}/app/gt-process.c
        ${CMAKE_SOURCE_DIR}/app/gt-process.h

        ${CMAKE_SOURCE_DIR}/app/gt-terminal.h
        ${CMAKE_SOURCE_DIR}/app/gt-terminal.c

        ${CMAKE_SOURCE_DIR}/app/gt-window.c
        ${CMAKE_SOURCE_DIR}/app/gt-window.h
        ${CMAKE_SOURCE_DIR}/app/gt-window.ui

        ${CMAKE_SOURCE_DIR}/app/gt-marshals.h
        ${CMAKE_SOURCE_DIR}/app/gt-marshals.c

        ${CMAKE_SOURCE_DIR}/app/gt-proxy-info.c
        ${CMAKE_SOURCE_DIR}/app/gt-proxy-info.h

        ${CMAKE_SOURCE_DIR}/app/gt-theme-switcher.h
        ${CMAKE_SOURCE_DIR}/app/gt-theme-switcher.c
        ${CMAKE_SOURCE_DIR}/app/gt-theme-switcher.ui

        ${CMAKE_SOURCE_DIR}/app/gt-util.h
        ${CMAKE_SOURCE_DIR}/app/gt-util.c

        ${CMAKE_SOURCE_DIR}/app/gt-resources.h
        ${CMAKE_SOURCE_DIR}/app/gt-resources.c

        ${CMAKE_SOURCE_DIR}/app/gt-simple-tab.h
        ${CMAKE_SOURCE_DIR}/app/gt-simple-tab.c
        ${CMAKE_SOURCE_DIR}/app/gt-simple-tab.ui

        ${CMAKE_SOURCE_DIR}/app/gt-tab-button.h
        ${CMAKE_SOURCE_DIR}/app/gt-tab-button.c
        ${CMAKE_SOURCE_DIR}/app/gt-tab-button.ui

        ${CMAKE_SOURCE_DIR}/app/gt-tab-switcher.h
        ${CMAKE_SOURCE_DIR}/app/gt-tab-switcher.c
        ${CMAKE_SOURCE_DIR}/app/gt-tab-switcher.ui

        ${CMAKE_SOURCE_DIR}/app/gt-tab-switcher-row.h
        ${CMAKE_SOURCE_DIR}/app/gt-tab-switcher-row.c
        ${CMAKE_SOURCE_DIR}/app/gt-tab-switcher-row.ui

        ${CMAKE_SOURCE_DIR}/app/gt-watcher.h
        ${CMAKE_SOURCE_DIR}/app/gt-watcher.c

        ${CMAKE_SOURCE_DIR}/app/rgba.h

        xdg-fm1.h
        xdg-fm1.c

        logo.txt

        menus.ui
        )

add_executable(graceful-terminator ${CMAKE_SOURCE_DIR}/app/main.c ${GT_SRC})
target_link_libraries(graceful-terminator
        ${VTE_LIBRARIES}
        ${GTK_LIBRARIES}
        ${GIO_LIBRARIES}
        ${GDS_LIBRARIES}
        ${GTOP_LIBRARIES}
        ${PCRE_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${ADWAITA_LIBRARIES}
        ${GIO_UNIX_LIBRARIES} -lm)

target_link_directories(graceful-terminator PRIVATE ${GLIB_INCLUDE_DIRS} ${VTE_INCLUDE_DIRS})

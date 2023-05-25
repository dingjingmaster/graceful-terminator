project(graceful-terminator-nautilus4-extensions C)

add_library(nautilus-menu-graceful-terminator MODULE nautilus-menu-module.c
        nautilus-menu-provider.c nautilus-menu-provider.h)
target_link_libraries(nautilus-menu-graceful-terminator PUBLIC ${NAUTILUS4_LIBRARIES})
target_include_directories(nautilus-menu-graceful-terminator PUBLIC ${NAUTILUS4_INCLUDE_DIRS})

install(TARGETS nautilus-menu-graceful-terminator DESTINATION /usr/lib/nautilus/extensions-4)

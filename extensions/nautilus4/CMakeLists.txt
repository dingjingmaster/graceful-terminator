project(graceful-terminator-nautilus4-extensions C)

add_library(nautilus-menu-graceful-terminator MODULE menu-module.c menu-terminator.h menu-terminator.c)
target_link_libraries(nautilus-menu-graceful-terminator PUBLIC ${NAUTILUS4_LIBRARIES})
target_include_directories(nautilus-menu-graceful-terminator PUBLIC ${NAUTILUS4_INCLUDE_DIRS})
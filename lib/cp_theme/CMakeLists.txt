include_directories(include)

add_library(cp_theme STATIC src/theme.c)
target_link_libraries(cp_theme ${LIBXML2_LIBRARIES})
target_link_libraries(cp_theme ${LibRSVG_LIBRARIES})
target_link_libraries(cp_theme ${GOBJECT_LIBRARIES})

add_executable(theme_example src/example.c)
target_link_libraries(theme_example cp_theme)
include_directories(include)

add_library(theme STATIC src/theme.c)
target_link_libraries(theme ${LIBXML2_LIBRARIES})
target_link_libraries(theme ${LibRSVG_LIBRARIES})
target_link_libraries(theme ${GOBJECT_LIBRARIES})

add_executable(theme_example src/example.c)
target_link_libraries(theme_example theme)
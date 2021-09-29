
include_directories(include)

add_library(design STATIC src/design.c)
target_link_libraries(design ${LIBXML2_LIBRARIES})
target_link_libraries(design ${LibRSVG_LIBRARIES})
target_link_libraries(design ${GOBJECT_LIBRARIES})
target_link_libraries(design ${B64_LIBRARIES})

add_executable(design_example src/example.c)
target_link_libraries(design_example design)


include_directories("include")
add_library(cp_design STATIC src/design.c)
target_link_libraries(cp_design ${LIBXML2_LIBRARIES})
target_link_libraries(cp_design ${LibRSVG_LIBRARIES})
target_link_libraries(cp_design ${GOBJECT_LIBRARIES})
target_link_libraries(cp_design ${B64_LIBRARIES})

add_executable(design_example src/example.c)
target_link_libraries(design_example cp_design)

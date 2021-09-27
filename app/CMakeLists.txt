file(GLOB app_src src/*.c )

add_executable(cheesypic  ${app_src})
target_link_libraries(cheesypic glfw)
target_link_libraries(cheesypic OpenGL::GL)
target_link_libraries(cheesypic ${Gphoto2_LIBRARIES})
target_link_libraries(cheesypic ${TurboJPEG_LIBRARIES})
target_link_libraries(cheesypic ${JSONC_LIBRARY} )
target_link_libraries(cheesypic ${LIBXML2_LIBRARIES})
target_link_libraries(cheesypic ${GTK_LIBRARIES})
target_link_libraries(cheesypic ${LibRSVG_LIBRARIES})
target_link_libraries(cheesypic ${B64_LIBRARIES})

target_include_directories(cheesypic PUBLIC include )

install(TARGETS     cheesypic
        DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES   ${CMAKE_SRC_DIR}/assets/default.theme.svg 
                ${CMAKE_SRC_DIR}/assets/template.design.svg 
                ${CMAKE_SRC_DIR}/configuration/cheesypic.conf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}")

install(FILES ${CMAKE_BINARY_DIR}/AmaticSC-Bold.ttf ${CMAKE_BINARY_DIR}/AmaticSC-Regular.ttf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ 
        DESTINATION "/usr/local/share/fonts/")


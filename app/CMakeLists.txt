file(GLOB app_src src/*.c )

# add_definitions(-DNO_CAM)
# add_definitions(-DFAST_MODE)

add_executable(cheesypic  ${app_src})
target_link_libraries(cheesypic glfw)
target_link_libraries(cheesypic OpenGL::GL)
target_link_libraries(cheesypic ${Gphoto2_LIBRARIES})
target_link_libraries(cheesypic ${TurboJPEG_LIBRARIES})
target_link_libraries(cheesypic ${JSONC_LIBRARY} )
target_link_libraries(cheesypic Threads::Threads)
target_link_libraries(cheesypic design)
target_link_libraries(cheesypic theme)

target_include_directories(cheesypic PUBLIC 
        include 
        ${CMAKE_SOURCE_DIR}/lib/design/include
        ${CMAKE_SOURCE_DIR}/lib/theme/include)

install(TARGETS     cheesypic
        DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES   ${CMAKE_SOURCE_DIR}/assets/default.theme.svg 
                ${CMAKE_SOURCE_DIR}/assets/template.design.svg 
                ${CMAKE_SOURCE_DIR}/configuration/cheesypic.conf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}")

install(DIRECTORY   ${CMAKE_SOURCE_DIR}/addons
        DIRECTORY_PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        FILE_PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}")

install(FILES ${CMAKE_BINARY_DIR}/AmaticSC-Bold.ttf ${CMAKE_BINARY_DIR}/AmaticSC-Regular.ttf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ 
        DESTINATION "/usr/local/share/fonts/")


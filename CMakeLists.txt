cmake_minimum_required(VERSION 3.10)
# set the project name and version
project(cheesypic   VERSION     1.0
                    LANGUAGES   C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

# add_definitions(-DNO_CAM)
# add_definitions(-DNO_PRINT)
# add_definitions(-DNO_SAVE)
# add_definitions(-DFAST_MODE)
# add_definitions(-DSTART_WINDOWED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY    https://github.com/glfw/glfw.git
  GIT_TAG           63da04e5ced93fcb87a20513acdff5d78b1166ff
)
FetchContent_MakeAvailable(glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# add_subdirectory(${glfw_SOURCE_DIR})

find_package(OpenGL REQUIRED)
find_package(Gphoto2 REQUIRED)
find_package(TurboJPEG REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(JSONC REQUIRED)
find_package(Glib REQUIRED)
find_package(LibRSVG REQUIRED)
find_package(B64 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

include_directories(. 
    ${Gphoto2_INCLUDE_DIRS} 
    ${TurboJPEG_INCLUDE_DIRS} 
    ${LIBXML2_INCLUDE_DIR} 
    ${LIBXML2_INCLUDE_DIRS} 
    ${JSONC_INCLUDE_DIR} 
    ${Glib_INCLUDE_DIRS}
    ${LibRSVG_INCLUDE_DIR}
    ${GDK-PixBuf_INCLUDE_DIR}
    ${Cairo_INCLUDE_DIR}
    ${B64_INCLUSDE_DIR}
    "${glfw_SOURCE_DIR}/deps"
    "include")

message(STATUS "glfw path: ${glfw_SOURCE_DIR}/deps" ...)

# PHOTOBOOTH
add_executable(cheesypic src/main.c src/capture_thread.c src/decode_thread.c src/render_thread.c src/logic.c src/lodepng.c src/printer.c src/design.c )
target_link_libraries(cheesypic glfw)
target_link_libraries(cheesypic OpenGL::GL)
target_link_libraries(cheesypic ${Gphoto2_LIBRARIES})
target_link_libraries(cheesypic ${TurboJPEG_LIBRARIES})
target_link_libraries(cheesypic ${JSONC_LIBRARY} )
target_link_libraries(cheesypic ${LIBXML2_LIBRARIES})
target_link_libraries(cheesypic ${GTK_LIBRARIES})
target_link_libraries(cheesypic ${LibRSVG_LIBRARIES})
target_link_libraries(cheesypic ${B64_LIBRARIES})

install(TARGETS     cheesypic
        DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY   theme
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}")

# RENDER EXAMPLE
add_executable(render_jpeg src/render_jpg.c)
target_link_libraries(render_jpeg  ${Gphoto2_LIBRARIES})
target_link_libraries(render_jpeg  ${TurboJPEG_LIBRARIES})
target_link_libraries(render_jpeg glfw)
target_link_libraries(render_jpeg OpenGL::GL)

# SVG RENDER EXAMPLE
add_executable(svg_render src/svg_to_png.c src/design.c src/lodepng.c)
target_link_libraries(svg_render ${GTK_LIBRARIES})
target_link_libraries(svg_render ${LIBXML2_LIBRARIES})
target_link_libraries(svg_render ${LibRSVG_LIBRARIES})
target_link_libraries(svg_render ${B64_LIBRARIES})

# GLES EXAMPLE
add_executable(triangles src/triangles.c)
target_link_libraries(triangles glfw)
target_link_libraries(triangles OpenGL::GL)

# GTK EXAMPLE
add_executable(gtk src/gtk_test.c)
target_link_libraries(gtk ${GTK_LIBRARIES})


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

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

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
    "glfw/deps"
    "include")



# PHOTOBOOTH
add_executable(photobooth src/main.c src/capture_thread.c src/decode_thread.c src/render_thread.c src/logic.c src/lodepng.c src/printer.c src/design.c )
target_link_libraries(photobooth glfw)
target_link_libraries(photobooth OpenGL::GL)
target_link_libraries(photobooth ${Gphoto2_LIBRARIES})
target_link_libraries(photobooth ${TurboJPEG_LIBRARIES})
target_link_libraries(photobooth ${JSONC_LIBRARY} )
target_link_libraries(photobooth ${LIBXML2_LIBRARIES})
target_link_libraries(photobooth ${GTK_LIBRARIES})
target_link_libraries(photobooth ${LibRSVG_LIBRARIES})
target_link_libraries(photobooth ${B64_LIBRARIES})

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
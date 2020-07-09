cmake_minimum_required(VERSION 3.10)
# set the project name and version
project(glfw_test   VERSION     1.0
                     LANGUAGES   C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

add_definitions(-DNO_CAM)
add_definitions(-DNO_PRINT)
add_definitions(-DNO_SAVE)
add_definitions(-DFAST_MODE)

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
    ${B64_INCLUSDE_DIR})

# PHOTOBOOTH
add_executable(myapp main.c glad.c capture_thread.c decode_thread.c render_thread.c logic.c lodepng.c printer.c design.c)
target_link_libraries(myapp ${Gphoto2_LIBRARIES})
target_link_libraries(myapp ${TurboJPEG_LIBRARIES})
target_link_libraries(myapp ${JSONC_LIBRARY} )
target_link_libraries(myapp ${LIBXML2_LIBRARIES})
target_link_libraries(myapp ${LibRSVG_LIBRARIES})
target_link_libraries(myapp ${GTK_LIBRARIES})
target_link_libraries(myapp ${B64_LIBRARIES})
target_link_libraries(myapp glfw)
target_link_libraries(myapp OpenGL::GL)

# RENDER EXAMPLE
add_executable(render_jpeg render_jpg.c glad.c)
target_link_libraries(render_jpeg  ${Gphoto2_LIBRARIES})
target_link_libraries(render_jpeg  ${TurboJPEG_LIBRARIES})
target_link_libraries(render_jpeg glfw)
target_link_libraries(render_jpeg OpenGL::GL)

# SVG RENDER EXAMPLE
add_executable(svg_render svg_to_png.c design.c lodepng.c)
target_link_libraries(svg_render ${GTK_LIBRARIES})
target_link_libraries(svg_render ${LIBXML2_LIBRARIES})
target_link_libraries(svg_render ${LibRSVG_LIBRARIES})
target_link_libraries(svg_render ${B64_LIBRARIES})
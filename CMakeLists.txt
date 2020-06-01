cmake_minimum_required(VERSION 3.10)
# set the project name and version
project(glfw_test   VERSION     1.0
                     LANGUAGES   C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

add_definitions(-DNO_CAM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

find_package(OpenGL REQUIRED)
find_package(Gphoto2 REQUIRED)
find_package(TurboJPEG REQUIRED)
find_package(LibXml2 REQUIRED)

include_directories(. ${Gphoto2_INCLUDE_DIRS} ${TurboJPEG_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIRS})

# PHOTOBOOTH
add_executable(myapp main.c glad.c capture_thread.c decode_thread.c render_thread.c logic.c lodepng.c)
target_link_libraries(myapp  ${Gphoto2_LIBRARIES})
target_link_libraries(myapp  ${TurboJPEG_LIBRARIES})
target_link_libraries(myapp glfw)
target_link_libraries(myapp OpenGL::GL)

# RENDER EXAMPLE
add_executable(render_jpeg render_jpg.c glad.c)
target_link_libraries(render_jpeg  ${Gphoto2_LIBRARIES})
target_link_libraries(render_jpeg  ${TurboJPEG_LIBRARIES})
target_link_libraries(render_jpeg glfw)
target_link_libraries(render_jpeg OpenGL::GL)

# SVG PARSER EXAMPLE
add_executable(svg_parse parse_svg.c)
target_link_libraries(svg_parse  ${LIBXML2_LIBRARIES})

# PRINTER EXAMPLE
add_executable(photo_print printer_thread.c)

cmake_minimum_required(VERSION 3.11)
# set the project name and version
project(cheesypic   VERSION     1.0
                    LANGUAGES   C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# check if we are building on a raspberry pi 4
# execute_process(COMMAND grep -o "Raspberry Pi 4" /proc/cpuinfo OUTPUT_VARIABLE IS_PI4)

# if(DEFINED IS_PI4)
#     message(STATUS "PI")
# else()
#     message(STATUS "NOT PI")
# endif()

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY    "https://github.com/glfw/glfw.git"
    GIT_TAG           63da04e5ced93fcb87a20513acdff5d78b1166ff
)
FetchContent_MakeAvailable(glfw)

file(DOWNLOAD "https://github.com/google/fonts/blob/5c3d8ef085f3084db38936d0dcd39a567dbc1e01/ofl/amaticsc/AmaticSC-Regular.ttf=true"  "${CMAKE_BINARY_DIR}/AmaticSC-Regular.ttf")
file(DOWNLOAD "https://github.com/google/fonts/blob/5c3d8ef085f3084db38936d0dcd39a567dbc1e01/ofl/amaticsc/AmaticSC-Bold.ttf?raw=true" "${CMAKE_BINARY_DIR}/AmaticSC-Bold.ttf")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# add_subdirectory(${glfw_SOURCE_DIR})
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(OpenGL REQUIRED)
find_package(Gphoto2 REQUIRED)
find_package(TurboJPEG REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(JSONC REQUIRED)
find_package(Glib REQUIRED)
find_package(GObject REQUIRED)
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
    ${GOBJECT_INCLUDE_DIR}
    ${LibRSVG_INCLUDE_DIR}
    ${GDK-PixBuf_INCLUDE_DIR}
    ${Cairo_INCLUDE_DIR}
    ${B64_INCLUSDE_DIR}
    "${glfw_SOURCE_DIR}/deps")

message(STATUS "glfw path: ${glfw_SOURCE_DIR}/deps" ...)

# add_subdirectory(app)
add_subdirectory(lib/design)
add_subdirectory(lib/theme)
add_subdirectory(app)



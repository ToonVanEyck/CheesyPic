cmake_minimum_required(VERSION 3.10)
# set the project name and version
project(glfw_test   VERSION     1.0
                     LANGUAGES   C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_executable(myapp main.c glad.c)

add_subdirectory(glfw)
add_subdirectory(Simple-OpenGL-Image-Library)

find_package(OpenGL REQUIRED)
find_package(Gphoto2)
find_package(TurboJPEG)

include_directories(. ${Gphoto2_INCLUDE_DIRS} ${TurboJPEG_INCLUDE_DIRS})

target_link_libraries(myapp  ${Gphoto2_LIBRARIES})
target_link_libraries(myapp  ${TurboJPEG_LIBRARIES})
target_link_libraries(myapp glfw)
target_link_libraries(myapp OpenGL::GL)
target_link_libraries(myapp SOIL)

add_executable(camera_fps_test camera_fps_test.c)
target_link_libraries(camera_fps_test  ${Gphoto2_LIBRARIES})
target_link_libraries(camera_fps_test  ${TurboJPEG_LIBRARIES})

add_executable(jpeg_fps_test jpeg_fps_test.c)
target_link_libraries(jpeg_fps_test  ${TurboJPEG_LIBRARIES})
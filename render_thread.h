#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

// #include <sys/poll.h>
// #include <sys/time.h>
// #include <sys/timerfd.h>
// #include <sys/signalfd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/mman.h>
#include <signal.h>
#include <time.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <pthread.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "decode_thread.h"
#include "shared_memory.h"
#include "linmath.h"

#define PR "\033[0;35m"

void start_render_thread(shared_memory_t *shared_memory);
void stop_render_thread(int dummy);

int init_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program,GLuint *resize_mat, GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo);
void cleanup_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program,GLuint *resize_mat, GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo);
void run_render_thread(shared_memory_t *shared_memory, GLFWwindow **window, GLuint program, GLuint resize_mat);

#endif
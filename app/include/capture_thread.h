#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>

#include <sys/poll.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define PC "\033[0;36m"
#ifndef LOG
#define LOG(...) do{fprintf(stderr,"capture: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

#include "decode_thread.h"
#include "shared_memory.h"

#define FRAME_RATE 25
#define FRAME_PERIOD (1000000000/FRAME_RATE)
#define SLOWDOWN 0

void start_capture_thread(shared_memory_t *shared_memory);
void stop_capture_thread(int dummy);

int init_capture_thread(struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera);
void clean_capture_thread(struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera);
int init_timer(struct pollfd *fds, int *numfd);
int init_camera(GPContext **ctx, Camera **camera);
void run_capture_thread(shared_memory_t *shared_memory, struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera);
int camera_error_handler(GPContext **ctx, Camera **camera, int error_code);

#endif
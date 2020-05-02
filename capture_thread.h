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
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void start_capture_thread(void *shmem);
void stop_capture_thread();

int init_capture_thread(struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera);
int init_timer(struct pollfd *fds, int *numfd);
int init_signal(struct pollfd *fds, int *numfd);
int init_camera(GPContext **ctx, Camera **camera);
void run_capture_thread(void *shmem, struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera);

#endif
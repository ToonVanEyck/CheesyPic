#ifndef DECODE_THREAD_T
#define DECODE_THREAD_T

// #include <sys/poll.h>
// #include <sys/time.h>
// #include <sys/timerfd.h>
// #include <sys/signalfd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/mman.h>
// #include <signal.h>
// #include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <turbojpeg.h>

#include "shared_memory.h"

#define PD "\033[0;34m"

void *start_decode_thread(void *shared_buffer);
void stop_decode_thread();

int init_decode_thread();
void run_decode_thread(jpeg_buffer_t *shared_buffer);

#endif
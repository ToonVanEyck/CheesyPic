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

void *start_decode_thread(void *shared_memory);
void stop_decode_thread();

int init_decode_thread(preview_buffer_t preview_buffer[]);
void run_decode_thread(shared_memory_t *shared_memory);

#endif
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

#define PR "\033[0;35m"
#ifndef LOG
#define LOG(...) do{fprintf(stderr,"render: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

#ifdef START_WINDOWED
    #define WINDOW 1
#else
    #define WINDOW 0
#endif

#include "decode_thread.h"
#include "shared_memory.h"
#include "linmath.h"


void start_render_thread(shared_memory_t *shared_memory);
void stop_render_thread(int dummy);

#endif
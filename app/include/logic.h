#ifndef LOGIC_H
#define LOGIC_H

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include <b64/cencode.h>

#define PL "\033[0;32m"
#ifndef LOG
#define LOG(...) do{fprintf(stderr,"logic: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

#include <pthread.h>

#include "shared_memory.h"
#include "design.h"
#include "theme.h"
#include "printer.h"
#include "config.h"

typedef struct {
    unsigned photo_counter;
    jpg_photo_t *jpg_capture;
}session_t; 


void init_logic();
void run_logic(shared_memory_t *shared_memory, config_t *config, session_t *session, printer_info_t *printer_info);


#endif
#ifndef PRINTER_THREAD_H
#define PRINTER_THREAD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include <json-c/json.h>

#include "shared_memory.h"

#define PP "\033[0;33m"

void *start_printer_thread(void *shared_memory);
void stop_printer_thread();

int init_printer_thread();
void run_printer_thread(shared_memory_t *shared_memory);

#endif
#ifndef PRINTER_H
#define PRINTER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include <json-c/json.h>

#include "shared_memory.h"

#define PP "\033[0;33m"

typedef enum{
    Idle,
    Printing,
}printing_status_t;

typedef struct{
    char name[16];
    printing_status_t status;
    unsigned int media_max;
    unsigned int media_remaining;
    unsigned int lifetime_prints;
}deck_info_t;

typedef struct{
    unsigned int connected;
    unsigned int nuf_of_decks;
    deck_info_t *deck;
}printer_info_t;

#define MAX_LEN 1024

int get_printer_driver_name(char **name);
int get_printer_stats_from_json(char *driver_name, printer_info_t *printer_info);
int get_printer_stats(char *driver_name, printer_info_t *printer_info);
int print_file(const char *file);
int is_printing_finished(char *driver_name, printer_info_t *printer_info);

#endif
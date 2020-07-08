#ifndef LOGIC_H
#define LOGIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

#include <b64/cencode.h>

#include "shared_memory.h"
#include "design.h"
#include "lodepng.h"
#include "printer.h"


#define PL "\033[0;32m"

typedef enum{
    image_png,
    image_jpg,
    movie_xyz
}countdown_type_t;

typedef struct{
    unsigned char *path;
    unsigned width;
    unsigned height;
    unsigned char *data;
}overlay_t;

typedef struct{
    overlay_t cd_mov;
}movie_t;

typedef struct{
    struct itimerval delay;
    overlay_t cd_3;
    overlay_t cd_2;
    overlay_t cd_1;
}image_t;

typedef union data{
    image_t image;
    movie_t movie;
}countdown_data_t;

typedef struct{
    countdown_type_t type;
    countdown_data_t data;
}countdown_t;

typedef struct{
    countdown_t countdown;
    overlay_t idle;
    overlay_t smile;
    overlay_t print;
    struct itimerval preview_time;
    unsigned char *preview_mirror;
    unsigned char *reveal_mirror;
    char *printer_driver_name;
    design_t design;
}photobooth_config_t;


typedef struct photobooth_session{
    unsigned photo_counter;
    unsigned char *current_jpg;
    unsigned char **capture_data; // array of char pointers
}photobooth_session_t; 


int init_logic(shared_memory_t *shared_memory, photobooth_config_t *config, photobooth_session_t *session, printer_info_t *printer_info);
int load_png_image(overlay_t *overlay);
void free_logic(photobooth_config_t *config, printer_info_t *printer_info);
int read_config(photobooth_config_t *config);
void run_logic(shared_memory_t *shared_memory, photobooth_config_t *config, photobooth_session_t *session, printer_info_t *printer_info);


#endif
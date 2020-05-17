#ifndef LOGIC_H
#define LOGIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shared_memory.h"
#include "lodepng.h"


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
    unsigned delay_ms;
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
}photobooth_config_t;


int init_logic(photobooth_config_t *pbc);
int load_png_image(overlay_t *overlay);
void free_config(photobooth_config_t *pbc);
int read_config(photobooth_config_t *pbc);
void run_logic(shared_memory_t *shared_memory,photobooth_config_t *pbc);

#endif
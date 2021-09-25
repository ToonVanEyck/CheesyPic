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

#include "shared_memory.h"
#include "design.h"
#include "theme.h"
#include "lodepng.h"
#include "printer.h"


typedef struct{
    struct itimerval countdown_delay;
    struct itimerval preview_time;
    unsigned char *mirror_liveview;
    unsigned char *mirror_preview;
    unsigned char printing_enabled;
    char *printer_driver_name;
    char *photo_output_directory;
    char *photo_output_name;
    theme_t theme;
    design_t design;
}photobooth_config_t;

typedef struct photobooth_session{
    unsigned photo_counter;
    unsigned char *current_jpg;
    unsigned char **capture_data; // array of char pointers
}photobooth_session_t; 


int init_logic(shared_memory_t *shared_memory, photobooth_config_t *config, photobooth_session_t *session, printer_info_t *printer_info, char *design_path, char *theme_path);
int load_png_image(overlay_t *overlay);
void free_logic(photobooth_config_t *config, printer_info_t *printer_info);
int read_config(photobooth_config_t *config, char *design_path, char *theme_path);
void run_logic(shared_memory_t *shared_memory, photobooth_config_t *config, photobooth_session_t *session, printer_info_t *printer_info);
int get_latest_design(char *dir_path,char *design_path);
int get_latest_theme(char *dir_path,char *theme_path);

#endif
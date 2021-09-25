#ifndef CONFIG_H
#define CONFIG_H

#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <wordexp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#define PL "\033[0;32m"
#ifndef LOG
#define LOG(...) do{fprintf(stderr,"config: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

#include "design.h"

#define CONFIG_FILE "/usr/local/etc/cheesypic/cheesypic.conf"

typedef struct{
    struct itimerval countdown_time;
    struct itimerval preview_time;
    unsigned char *mirror_liveview;
    unsigned char *mirror_preview;
    unsigned char printing_enabled;
    char *printer_driver_name;
    char *photo_output_directory;
    char *photo_output_name;
    // cp_theme_t theme;
    cp_design_t cp_design;
}cp_config_t;

int read_config(cp_config_t *cp_config);
int get_recent_file_in_dir(char **file, char *directory, char *file_extention);

#endif
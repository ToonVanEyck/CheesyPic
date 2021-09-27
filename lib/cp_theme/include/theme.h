#ifndef THEME_H
#define THEME_H

#include <stdio.h>
#include <stdlib.h>

#include <librsvg-2.0/librsvg/rsvg.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <gobject/gobject.h>

// #include "lodepng.h"

#ifndef LOG
#define LOG(...) do{fprintf(stderr,"theme: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

typedef struct{
    unsigned width;
    unsigned height;
    unsigned char *data;
}overlay_t;

typedef struct{
    overlay_t cd_3;
    overlay_t cd_2;
    overlay_t cd_1;
    overlay_t push;
    overlay_t smile;
    overlay_t fail;
    overlay_t print;
}theme_t;

int load_theme_from_file(theme_t *theme, const char *svg_theme);
void free_theme(theme_t *theme);

#endif
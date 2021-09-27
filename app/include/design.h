#ifndef DESIGN_H
#define DESIGN_H

#include <stdio.h>
#include <stdlib.h>

#include <librsvg-2.0/librsvg/rsvg.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <gobject/gobject.h>

#include "lodepng.h"

#ifndef LOG
#define LOG(...) do{fprintf(stderr,"design: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

typedef struct{
    xmlNode *photo_node;
    unsigned id;
}photo_element_t;

typedef struct{
    unsigned total_photos; // number of unique photos to be made
    unsigned total_locations; // number of locations where photos will be placed
    photo_element_t *photo_list;
}cp_design_t;

int load_design_from_file(cp_design_t *design, const char *svg_design);
int render_design(cp_design_t *design, unsigned char **capture_data);
void free_design(cp_design_t *design);

#endif
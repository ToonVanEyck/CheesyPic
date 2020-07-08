#ifndef DESIGN_H
#define DESIGN_H

#include <stdio.h>
#include <stdlib.h>

#include <librsvg-2.0/librsvg/rsvg.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <gobject/gobject.h>

#include "lodepng.h"

typedef struct{
    xmlNode *photo_node;
    unsigned id;
}photo_element_t;

typedef struct{
    unsigned total_photos; // number of unique photos to be made
    unsigned total_locations; // number of locations where photos will be placed
    photo_element_t *photo_list;
    xmlNode *root;
    xmlDoc *doc;
}design_t;

int load_design_from_file(design_t *design, const char *svg_design);
int render_design(design_t *design, unsigned char **capture_data);
void free_design(design_t *design);

#endif
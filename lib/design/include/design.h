#ifndef DESIGN_H
#define DESIGN_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <glib-object.h>
#include <librsvg-2.0/librsvg/rsvg.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <b64/cencode.h>

#ifndef LOG
#define LOG(...) do{fprintf(stderr,"design: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

typedef struct{
    size_t size;
    unsigned char *data;
}jpg_photo_t;

typedef struct{
    xmlNode *photo_node;
    unsigned id;
}photo_element_t;

typedef struct{
    unsigned total_photos; // number of unique photos to be made
    unsigned total_locations; // number of locations where photos will be placed
    photo_element_t *photo_list;
}design_t;

int load_design_from_file(design_t *design, const char *svg_design);
int render_design(design_t *design, jpg_photo_t *jpg_photo);
void free_design(design_t *design);

#endif
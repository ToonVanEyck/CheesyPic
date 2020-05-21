#ifndef PARSE_SVG_H
#define PARSE_SVG_H

#include <stdio.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

typedef struct{
    unsigned char id;
    double x;
    double y;
    double width;
    double height;
    char transformation_string[1024];
}photo_location_t;

void print_photo_location(photo_location_t *photo_location);
int cnt_photos(xmlNode * a_node);
void append_photo_node_to_list(xmlNode * a_node,xmlNode ***photo_nodes, int *index);
void get_photos(xmlNode * a_node, xmlNode ***photo_nodes, int *num_photos);
xmlNode *xml_sibling_node_by_name(xmlNode * a_node, const char *name);
xmlNode *xml_child_node_by_name(xmlNode * a_node, const char *name);
xmlNode *xml_get_node_where_attribute_has_value(xmlNode * a_node, const char *attribute_key, const char *attribute_value);

#endif
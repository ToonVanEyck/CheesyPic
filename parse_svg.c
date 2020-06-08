#include "parse_svg.h"

void print_photo_location(photo_location_t *photo_location)
{
    printf("-----------------------------------------------------------------------------------\n");
    printf("id     : %d\n",photo_location->id);
    printf("x      : %f\n",photo_location->x);
    printf("y      : %f\n",photo_location->y);
    printf("width  : %f\n",photo_location->width);
    printf("height : %f\n",photo_location->height);
    for(int i = 0;photo_location->transformation_string[i] != NULL;i++){
        printf("trans  : %s\n",photo_location->transformation_string[i]);
    }
    printf("-----------------------------------------------------------------------------------\n");
}
int xml_node_contains_child_with_name(xmlNode * a_node, const char *name)
{
    for (xmlNode *child_node = a_node->children; child_node; child_node = child_node->next) {
        if (!strcmp(child_node->name,name)) {
            return 1;
        }
    }
    return 0;
}
int cnt_photos(xmlNode * a_node)
{
    int cnt = 0;
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_contains_child_with_name(cur_node, "rect") && xml_node_contains_child_with_name(cur_node, "text")){
            cnt++;
        }
        cnt += cnt_photos(cur_node->children);
    }
    return cnt;
}
void append_photo_node_to_list(xmlNode * a_node,xmlNode ***photo_nodes, int *index)
{
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_contains_child_with_name(cur_node, "rect") && xml_node_contains_child_with_name(cur_node, "text")){
            (*photo_nodes)[(*index)++] = cur_node;
        }
        append_photo_node_to_list(cur_node->children,photo_nodes,index);
    }
}
void get_photos(xmlNode * a_node, xmlNode ***photo_nodes, int *num_photos)
{
    *num_photos = cnt_photos(a_node);
    *photo_nodes = malloc((*num_photos)*sizeof(xmlNode*)); 
    int i = 0;
    append_photo_node_to_list(a_node,photo_nodes,&i);
}
xmlNode *xml_sibling_node_by_name(xmlNode * a_node, const char *name)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (!strcmp(cur_node->name,name)) {
            return cur_node;
        }
    }
    return NULL;
}
xmlNode *xml_child_node_by_name(xmlNode * a_node, const char *name)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next) {
        if (!strcmp(cur_node->name,name)) {
            return cur_node;
        }
    }
    return NULL;
}
xmlNode *xml_get_node_where_attribute_has_value(xmlNode * a_node, const char *attribute_key, const char *attribute_value)
{
    for (xmlNode *child_node = a_node->children; child_node; child_node = child_node->next) {
        if (child_node->type == XML_ELEMENT_NODE) {
            xmlChar *label = xmlGetProp(child_node,attribute_key);
            if(label && !strcmp(label,attribute_value)){
                return child_node;
            }
        }
    }
    return NULL;
}

int get_photo_locations_from_design(const char * svg_file,photo_locations_t *photo_locations)
{
    xmlParserCtxtPtr ctxt; /* the parser context */
    xmlDocPtr doc; /* the resulting document tree */

    /* create a parser context */
    ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
        fprintf(stderr, "Failed to allocate parser context\n");
	return 1;
    }
    /* parse the file, activating the DTD validation option */
    doc = xmlReadFile(svg_file, NULL, 0);
    /* check if parsing suceeded */
    if(doc == NULL){
        xmlFreeDoc(doc);
        return -1;
    }
    xmlNode *root_element = xmlDocGetRootElement(doc);
    if(root_element == NULL){
        xmlFreeDoc(doc);
        return -1;
    }
    xmlNode *svg_element = xml_sibling_node_by_name(root_element,"svg");
    if(svg_element == NULL){
        xmlFreeDoc(doc);
        return -1;
    }
    xmlNode *photos_node = xml_get_node_where_attribute_has_value(svg_element,"label","photos");
    if(photos_node == NULL){
        xmlFreeDoc(doc);
        return -1;
    }
   
    xmlNode **photo_nodes;
    get_photos(photos_node, &photo_nodes, &photo_locations->total_photos);
    photo_locations->photo_location = malloc(photo_locations->total_photos * sizeof(photo_location_t));
    memset(photo_locations->photo_location,0,photo_locations->total_photos * sizeof(photo_location_t));
    for(int i = 0; i<photo_locations->total_photos ; i++){
        xmlNode *text_node = xml_child_node_by_name(photo_nodes[i],"text");
        xmlNode *rect_node = xml_child_node_by_name(photo_nodes[i],"rect");
        photo_locations->photo_location[i].id = strtol(text_node->children->children->content,NULL,10);
        photo_locations->photo_location[i].x = strtod(xmlGetProp(rect_node,"x"),NULL);
        photo_locations->photo_location[i].y = strtod(xmlGetProp(rect_node,"y"),NULL);
        photo_locations->photo_location[i].width  = strtod(xmlGetProp(rect_node,"width"),NULL);
        photo_locations->photo_location[i].height = strtod(xmlGetProp(rect_node,"height"),NULL);
        int transform_i = 0;
        for(xmlNode *cur_node = rect_node ; cur_node != svg_element ; cur_node = cur_node->parent){
            char *transform = xmlGetProp(cur_node,"transform");
            if(transform){
                if(strlen(transform)+1<256){
                    photo_locations->photo_location[i].transformation_string[transform_i] = malloc(strlen(transform)+1);
                    strcpy(photo_locations->photo_location[i].transformation_string[transform_i],transform);
                    if(++transform_i>=MAX_TRANSFORMS){
                        fprintf(stderr,"Error to many transforms!\n");
                        return -1; 
                    }
                }else{
                    fprintf(stderr,"Error transformation string to long!\n");
                    return -1;
                }
            }
        }
        print_photo_location(&photo_locations->photo_location[i]);
    }
	xmlFreeDoc(doc);
    return photo_locations->total_photos;
}

void free_photo_locations(photo_locations_t *photo_locations)
{

    for(int i = 0;i<photo_locations->total_photos;i++)
    {
        for(int j =0;photo_locations->photo_location[i].transformation_string[j] != NULL ; j++){
            free(photo_locations->photo_location[i].transformation_string[j]);
        }
    }
    free(photo_locations->photo_location);
}
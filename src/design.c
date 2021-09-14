#include "design.h"

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
int xml_node_is_cheesypic_placeholder(xmlNode * a_node)
{
    int rc = 0;
    if (a_node->type == XML_ELEMENT_NODE) {
        xmlChar *id = xmlGetProp(a_node,"id");
        if(id && strstr(id,"cheesypic_placeholder")){
            rc = 1;
        }
        xmlFree(id);
    }
    return rc;
}

int xml_node_is_cheesypic_cutline(xmlNode * a_node)
{
    int rc = 0;
    if (a_node->type == XML_ELEMENT_NODE) {
        xmlChar *id = xmlGetProp(a_node,"id");
        if(id && strstr(id,"cheesypic_cutline")){
            rc = 1;
        }
        xmlFree(id);
    }
    return rc;
}

int cnt_locations(xmlNode * a_node)
{
    int cnt = 0;
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_is_cheesypic_placeholder(cur_node)){//xml_node_contains_child_with_name(cur_node, "rect") && xml_node_contains_child_with_name(cur_node, "text")){
            cnt++;
        }
        cnt += cnt_locations(cur_node->children);
    }
    return cnt;
}

void cnt_photos(xmlNode * a_node, unsigned *total_photos)
{
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_is_cheesypic_placeholder(cur_node)){//xml_node_contains_child_with_name(cur_node, "rect") && xml_node_contains_child_with_name(cur_node, "text")){
            xmlNode *text_node = xml_child_node_by_name(cur_node,"text");
            if(text_node){
                unsigned id = strtol(text_node->children->content,NULL,10);
                if(id > *total_photos) *total_photos = id;
            }
        }
        cnt_photos(cur_node->children, total_photos);
    }
}

void prepare_photo_list(xmlNode * a_node,photo_element_t *photo_list, int *index)
{
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_is_cheesypic_placeholder(cur_node)){
            xmlNode *child_node,*next_node;
            next_node = cur_node->children;
            while(next_node){
                child_node = next_node;
                next_node = child_node->next;
                if(!strcmp(child_node->name,"image")){
                    photo_list[*index].photo_node = child_node;
                }else{
                    if(!strcmp(child_node->name,"text") && child_node->type == XML_ELEMENT_NODE){
                        char *content = xmlNodeGetContent(child_node);
                        photo_list[*index].id = strtol(content,NULL,10);
                        xmlFree(content);  
                        (*index)++;
                    }
                    xmlUnlinkNode(child_node);
                    xmlFreeNode(child_node);
                }
            }
        }
        prepare_photo_list(cur_node->children,photo_list,index);
        if(xml_node_is_cheesypic_cutline(cur_node)){
            xmlUnlinkNode(cur_node);
            xmlFreeNode(cur_node);
        }
    }
}

int load_design_from_file(design_t *design, const char *svg_design)
{
    memset(design,0,sizeof(design_t));
    design->doc = xmlReadFile(svg_design, NULL, 0);
    if(design->doc == NULL){
        // xmlFreeDoc(design->doc); // file is freed elsewhere
        return 1;
    }
    design->root = xmlDocGetRootElement(design->doc);
    if(design->root == NULL){
        // xmlFreeDoc(design->doc); // file is freed elsewhere
        return 1;
    }

    design->total_locations = cnt_locations(design->root);
    cnt_photos(design->root,&design->total_photos);
    LOG("%d unique photos spread over %d locations\n",design->total_photos,design->total_locations);

    design->photo_list = malloc(design->total_locations*sizeof(photo_element_t));
    if(design->photo_list == NULL){
        LOG("failled to allocate memory for photo list\n");
        return 1;
    }
    memset(design->photo_list,0,design->total_locations*sizeof(photo_element_t*));
    int i = 0;
    prepare_photo_list(design->root,design->photo_list,&i);

    return 0;
}

int render_design(design_t *design, unsigned char **capture_data)
{
    // REPLACE IMAGES!!
    xmlNs *xlink_ns = NULL;
    for (xmlNs *cur_ns = design->root->nsDef; cur_ns; cur_ns = cur_ns->next) {
        if(cur_ns->prefix && !strcmp(cur_ns->prefix,"xlink")){
            xlink_ns = cur_ns;
            break;
        }
    }
    if(xlink_ns == NULL){
        LOG("Couldn't find xlink namespace\n");
        return 1;
    }

    for(int i = 0;i<design->total_locations;i++){
        xmlSetNsProp(design->photo_list[i].photo_node,xlink_ns,"href",capture_data[design->photo_list[i].id-1]);
    }

    xmlBuffer *buffer = xmlBufferCreate();
    xmlNodeDump(buffer,design->doc,design->root,0,0);
    RsvgHandle *handle;
    GError *error = NULL;
    RsvgDimensionData dim;
    double width, height;
    cairo_surface_t *design_surface;
    cairo_t *design_ctx;
    cairo_status_t status;

    // FILE *fp = fopen("design.svg", "w");
    // fLOG(fp,"%s",xmlBufferContent(buffer));
    // fclose(fp);

    //handle = rsvg_handle_new_from_data(xmlBufferContent(buffer),xmlBufferLength(buffer),&error);
    handle = rsvg_handle_new_with_flags(RSVG_HANDLE_FLAG_UNLIMITED);
    if(handle == NULL){
        LOG("failed to create handle from data\n");
        exit(1);
    }
    rsvg_handle_write(handle,xmlBufferContent(buffer),xmlBufferLength(buffer),&error);
    if (error != NULL)
    {
        LOG("rsvg_handle_new_from_file error!\n");
        LOG ("%s\n",error->message);
        return 1;
    }
    if(!rsvg_handle_close(handle,&error)){
        LOG("failed to close the handle!\n");
        return 1;
    }
    rsvg_handle_set_dpi_x_y(handle,300.0,300.0);
    rsvg_handle_get_dimensions (handle, &dim);
    width = dim.width;
    height = dim.height;

    design_surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
    design_ctx = cairo_create(design_surface);

    cairo_set_source_rgb(design_ctx, 1, 1, 1);
    cairo_paint(design_ctx);

    rsvg_handle_render_cairo(handle, design_ctx);
    status = cairo_status (design_ctx);
    if (status)
    {
        LOG("cairo_status!\n");
        LOG ("%s\n",cairo_status_to_string (status));
        return 1;
    }

    cairo_surface_write_to_png (design_surface, "print_me.png");

    cairo_destroy (design_ctx);
    cairo_surface_destroy (design_surface);
    //g_object_unref(handle);
    xmlBufferFree(buffer);
    return 1;
}

void free_design(design_t *design)
{
    xmlCleanupParser();
    free(design->photo_list);
    //xmlFreeNode(design->root);
    xmlFreeDoc(design->doc);

}
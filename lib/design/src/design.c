#include "design.h"

static xmlNode *root; 
static xmlDoc *doc;

static struct timeval start, encoded, rendered;

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
        xmlChar *is_placeholder = xmlGetProp(a_node,"cheesypic_placeholder");
        if(is_placeholder){
            rc = 1;
        }
        xmlFree(is_placeholder);
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
        if(xml_node_is_cheesypic_placeholder(cur_node)){
            cnt++;
        }
        cnt += cnt_locations(cur_node->children);
    }
    return cnt;
}

void cnt_photos(xmlNode * a_node, unsigned *total_photos)
{
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(xml_node_is_cheesypic_placeholder(cur_node)){
            xmlNode *text_node = xml_child_node_by_name(cur_node,"text");
            if(text_node){
                unsigned id = strtol(text_node->children->content,NULL,10);
                if(id > *total_photos) *total_photos = id;
            }
        }
        cnt_photos(cur_node->children, total_photos);
    }
}

unsigned char * encode_jpg_for_svg(jpg_photo_t jpg_photo){
    int encoded_size = 32 + 2*jpg_photo.size;
    unsigned char *encoded_data = malloc(encoded_size);
    if(jpg_photo.data != NULL){
        strcpy(encoded_data,"data:image/jpeg;base64,");
        char* c = encoded_data+23;
        int cnt = 0;
        base64_encodestate s;
        base64_init_encodestate(&s);
        cnt = base64_encode_block(jpg_photo.data, jpg_photo.size, c, &s);
        c += cnt;
        cnt = base64_encode_blockend(c, &s);
        c += cnt;
        *c = 0;
    }
    return encoded_data;
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
    doc = xmlReadFile(svg_design, NULL, 0);
    if(doc == NULL){
        return 1;
    }
    root = xmlDocGetRootElement(doc);
    if(root == NULL){
        return 1;
    }

    design->total_locations = cnt_locations(root);
    cnt_photos(root,&design->total_photos);
    LOG("%d unique photos spread over %d locations\n",design->total_photos,design->total_locations);

    design->photo_list = malloc(design->total_locations*sizeof(photo_element_t));
    if(design->photo_list == NULL){
        LOG("failled to allocate memory for photo list\n");
        return 1;
    }
    memset(design->photo_list,0,design->total_locations*sizeof(photo_element_t*));
    int i = 0;
    prepare_photo_list(root,design->photo_list,&i);
    return 0;
}

int render_design(design_t *design, jpg_photo_t *jpg_photo)
{
    // REPLACE IMAGES!!
    xmlNs *xlink_ns = NULL;
    for (xmlNs *cur_ns = root->nsDef; cur_ns; cur_ns = cur_ns->next) {
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
        unsigned char *encoded_jpg = encode_jpg_for_svg(jpg_photo[design->photo_list[i].id-1]);
        if(encoded_jpg != NULL){
            xmlSetNsProp(design->photo_list[i].photo_node,xlink_ns,"href",encoded_jpg);
            free(encoded_jpg);
        }
    }
    xmlBuffer *buffer = xmlBufferCreate();
    xmlNodeDump(buffer,doc,root,0,0);
    RsvgHandle *handle = NULL;
    GError *error = NULL;
    RsvgDimensionData dim;
    double width, height;
    cairo_surface_t *design_surface;
    cairo_t *design_ctx;
    cairo_status_t status;

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
    cairo_surface_flush(design_surface);
    cairo_surface_write_to_png (design_surface, "print_me.png");

    cairo_destroy(design_ctx);
    cairo_surface_finish(design_surface);
    cairo_surface_destroy (design_surface);
    g_object_unref(handle);
    xmlBufferFree(buffer);
    return 1;
}

void free_design(design_t *design)
{
    free(design->photo_list);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}
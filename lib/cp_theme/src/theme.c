#include <librsvg-2.0/librsvg/rsvg.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <b64/cencode.h>

#include "theme.h"

#ifndef LOG
#define LOG(...) do{fprintf(stderr,"theme: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

static xmlNode *cd_3_node = NULL;
static xmlNode *cd_2_node = NULL;
static xmlNode *cd_1_node = NULL;
static xmlNode *push_node = NULL;
static xmlNode *smile_node = NULL;
static xmlNode *fail_node = NULL;
static xmlNode *print_node = NULL;
static xmlNode *bg_transparent_node = NULL;
static xmlNode *bg_opaque_node = NULL;
static xmlNode *fg_overlay_node = NULL;

static xmlNode *root;
static xmlDoc *doc;

xmlNode * xml_get_child_theme_layer_by_name(xmlNode * a_node,const xmlChar *layer_name)
{
    xmlNode *layer_node = NULL;
    for (xmlNode *cur_node = a_node->children; cur_node && !layer_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            xmlChar *label = xmlGetProp(cur_node,"label");
            if(label && strstr(layer_name, label)){
                layer_node = cur_node;
            }
            xmlFree(label);
        }
    }
    return layer_node;
}

void xml_theme_layer_set_visability(xmlNode * layer_node, int make_visable)
{
    if (layer_node->type == XML_ELEMENT_NODE) {
        xmlSetProp(layer_node,"style",make_visable?"display:inline":"display:none");
    }
}

int render_theme_layer(theme_t *theme, xmlNode *layer, overlay_t *overlay)
{
    // xml_theme_layer_set_visability(cd_3_node, 0);
    // xml_theme_layer_set_visability(cd_2_node, 0);
    // xml_theme_layer_set_visability(cd_1_node, 0);
    // xml_theme_layer_set_visability(push_node, 0);
    // xml_theme_layer_set_visability(smile_node, 0);
    // xml_theme_layer_set_visability(fail_node, 0);
    // xml_theme_layer_set_visability(print_node, 0);
    // xml_theme_layer_set_visability(bg_transparent_node, 0);
    // xml_theme_layer_set_visability(bg_opaque_node, 0);
    // xml_theme_layer_set_visability(fg_overlay_node, 1);

    xml_theme_layer_set_visability(layer, 1);
    if (layer->type == XML_ELEMENT_NODE) {
        xmlChar *background = xmlGetProp(layer,"cheesypic_background");
        if(background){
            if(strstr(background,"bg_transparent")) xml_theme_layer_set_visability(bg_transparent_node, 1);
            if(strstr(background,"bg_opaque")) xml_theme_layer_set_visability(bg_opaque_node, 1);
        }
        xmlFree(background);
    }

    xmlBuffer *buffer = xmlBufferCreate();
    xmlNodeDump(buffer,doc,root,0,0);
    RsvgHandle *handle;
    GError *error = NULL;
    RsvgDimensionData dim;
    double width, height;
    cairo_surface_t *theme_surface;
    cairo_t *theme_ctx;
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

    theme_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    theme_ctx = cairo_create(theme_surface);

    cairo_set_source_rgba(theme_ctx, 0, 0, 0, 0);
    cairo_paint(theme_ctx);
    rsvg_handle_render_cairo(handle, theme_ctx);
    status = cairo_status (theme_ctx);
    if (status)
    {
        LOG("cairo_status!\n");
        LOG ("%s\n",cairo_status_to_string (status));
        return 1;
    }

    overlay->height = cairo_image_surface_get_height(theme_surface);
    overlay->width = cairo_image_surface_get_width(theme_surface);
    size_t img_size = (4 * overlay->width * overlay->height);
    overlay->data = malloc(img_size);
    if(!overlay->data){
        LOG("Failed to allocate data for theme image!\n");
        return 1;
    }
    cairo_surface_flush(theme_surface);
    unsigned char *theme_layer_data = cairo_image_surface_get_data(theme_surface);
    for(size_t i=0; i<img_size;i+=4){
        (overlay->data)[i+0] = theme_layer_data[i+2]; //red
        (overlay->data)[i+1] = theme_layer_data[i+1]; //green
        (overlay->data)[i+2] = theme_layer_data[i+0]; //blue
        (overlay->data)[i+3] = theme_layer_data[i+3]; //alfa
    }
    cairo_destroy (theme_ctx);
    cairo_surface_finish(theme_surface);
    cairo_surface_destroy (theme_surface);
    g_object_unref(handle);
    xmlBufferFree(buffer);
    return 0;
}

int load_theme_from_file(theme_t *theme, const char *svg_theme)
{
    memset(theme,0,sizeof(theme_t));
    doc = xmlReadFile(svg_theme, NULL, 0);
    if(doc == NULL){
        return 1;
    }
    root = xmlDocGetRootElement(doc);
    if(root == NULL){
        return 1;
    }

    cd_3_node = xml_get_child_theme_layer_by_name(root,"3");
    cd_2_node = xml_get_child_theme_layer_by_name(root,"2");
    cd_1_node = xml_get_child_theme_layer_by_name(root,"1");
    push_node = xml_get_child_theme_layer_by_name(root,"push");
    smile_node = xml_get_child_theme_layer_by_name(root,"smile");
    fail_node = xml_get_child_theme_layer_by_name(root,"fail");
    print_node = xml_get_child_theme_layer_by_name(root,"print");
    bg_transparent_node = xml_get_child_theme_layer_by_name(root,"bg_transparent");
    bg_opaque_node = xml_get_child_theme_layer_by_name(root,"bg_opaque");
    fg_overlay_node = xml_get_child_theme_layer_by_name(root,"fg_overlay");

    if(!cd_3_node){           LOG("No cd_3 layer found in %s\n",svg_theme); return 1;}
    if(!cd_2_node){           LOG("No cd_2 layer found in %s\n",svg_theme); return 1;}
    if(!cd_1_node){           LOG("No cd_1 layer found in %s\n",svg_theme); return 1;}
    if(!push_node){           LOG("No push layer found in %s\n",svg_theme); return 1;}
    if(!smile_node){          LOG("No smile layer found in %s\n",svg_theme); return 1;}
    if(!fail_node){           LOG("No fail layer found in %s\n",svg_theme); return 1;}
    if(!print_node){          LOG("No print layer found in %s\n",svg_theme); return 1;}
    if(!bg_transparent_node){ LOG("No bg_transparent layer found in %s\n",svg_theme); return 1;}
    if(!bg_opaque_node){      LOG("No bg_opaque layer found in %s\n",svg_theme); return 1;}
    if(!fg_overlay_node){     LOG("No fg_overlay layer found in %s\n",svg_theme); return 1;}
    
    if(render_theme_layer(theme,cd_3_node, &theme->cd_3 )){  LOG("Couldn't render theme->cd_3 \n"); return 1;}
    if(render_theme_layer(theme,cd_2_node, &theme->cd_2 )){  LOG("Couldn't render theme->cd_2 \n"); return 1;}
    if(render_theme_layer(theme,cd_1_node, &theme->cd_1 )){  LOG("Couldn't render theme->cd_1 \n"); return 1;}
    if(render_theme_layer(theme,push_node, &theme->push )){  LOG("Couldn't render theme->push \n"); return 1;}
    if(render_theme_layer(theme,smile_node,&theme->smile )){ LOG("Couldn't render theme->smile\n"); return 1;}
    if(render_theme_layer(theme,fail_node, &theme->fail )){  LOG("Couldn't render theme->fail \n"); return 1;}
    if(render_theme_layer(theme,print_node,&theme->print )){ LOG("Couldn't render theme->print\n"); return 1;}

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}

void free_theme(theme_t *theme)
{
    free(theme->cd_3.data);
    free(theme->cd_2.data);
    free(theme->cd_1.data);
    free(theme->push.data);
    free(theme->smile.data);
    free(theme->fail.data);
    free(theme->print.data);
}

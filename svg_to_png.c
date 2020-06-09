#include <librsvg-2.0/librsvg/rsvg.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
//#include <glib/gprintf.h>

#include "parse_svg.h"
#include "shared_memory.h"
#include "lodepng.h"


#define SVGFILE "../2x6 Ruben Dani.svg"

#define UNKNO 0
#define MATRX 1
#define TRANS 2
#define SCALE 3
#define ROTAT 4
#define INVRT 5 // not supported for now
#define SKEWY 6 // not supported for now
#define SKEWX 7 // not supported for now

void str_to_matrix(char *string,cairo_matrix_t *matrix){
    int transform_type = 0;
    char *token = strtok(string,"(,)");

    if(!transform_type)transform_type = !strcmp("matrix",token)?MATRX:UNKNO;
    if(!transform_type)transform_type = !strcmp("translate",token)?TRANS:UNKNO;
    if(!transform_type)transform_type = !strcmp("scale",token)?SCALE:UNKNO;
    if(!transform_type)transform_type = !strcmp("rotate",token)?ROTAT:UNKNO;
    double vars[6] = {0};
    for(int i = 0;i<6;i++) {
        token = strtok(NULL,"(,)");
        if(token == NULL) break;
        vars[i] = strtod(token,NULL);
    }
    cairo_matrix_init_identity(matrix);
    switch (transform_type){
        case MATRX:
            matrix->xx = vars[0];
            matrix->yx = vars[1];
            matrix->xy = vars[2];
            matrix->yy = vars[3];
            matrix->x0 = vars[4];
            matrix->y0 = vars[5];
            break;
        case TRANS:
            cairo_matrix_translate(matrix,vars[0],vars[1]);
        break;
        case SCALE:
            cairo_matrix_scale(matrix,vars[0],vars[1]);
        break;
        case ROTAT:
            cairo_matrix_translate(matrix,vars[1],vars[2]);
            cairo_matrix_rotate(matrix,vars[0]*(G_PI/180));
            cairo_matrix_translate(matrix,-vars[1],-vars[2]);
        break;
        default:
            fprintf(stderr,"unknown transformation [%s]\n",string);
            break;
    }
}

void paint_photo_on_design(int img_width, int img_height, unsigned char *img_data, photo_location_t *location, cairo_t *design_ctx)
{
    cairo_matrix_t current_matrix,result_matrix,transform_matrix;
    cairo_matrix_init_identity(&current_matrix);
    cairo_matrix_init_identity(&result_matrix);
    cairo_matrix_init_identity(&transform_matrix);
    //transform photo to original svg rect
    cairo_matrix_translate(&current_matrix, location->x, location->y);
    cairo_matrix_scale(&current_matrix,location->width/(double)img_width,location->height/(double)img_height);
        printf("matrix(%f,%f,%f,%f,%f,%f)\n",
            current_matrix.xx,
            current_matrix.yx,
            current_matrix.xy,
            current_matrix.yy,
            current_matrix.x0,
            current_matrix.y0);
    for(int i = 0;location->transformation_string[i]!=NULL;i++){
        str_to_matrix(location->transformation_string[i], &transform_matrix);
        printf("matrix(%f,%f,%f,%f,%f,%f)\n",
            transform_matrix.xx,
            transform_matrix.yx,
            transform_matrix.xy,
            transform_matrix.yy,
            transform_matrix.x0,
            transform_matrix.y0);
        cairo_matrix_multiply (&result_matrix,&current_matrix,&transform_matrix);
        memcpy(&current_matrix,&result_matrix,sizeof(cairo_matrix_t));
    }

    cairo_set_matrix(design_ctx,&result_matrix);
    cairo_surface_t *photo_surface = cairo_image_surface_create_for_data (img_data,CAIRO_FORMAT_RGB24,img_width,img_height,img_width*4);
    cairo_set_source_surface(design_ctx,photo_surface,00,00);
    cairo_paint(design_ctx);
    cairo_surface_destroy(photo_surface);
}


int load_png_image(const char *file, unsigned *width, unsigned *height, unsigned char **data)
{
    unsigned error = 0;

    unsigned char* png = 0;
    size_t pngsize;

    error = lodepng_load_file(&png, &pngsize, file);
    if(!error) error = lodepng_decode32(data, width, height, png, pngsize);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    free(png);
    return error;
}


int main()
{
    photo_locations_t photo_locations;
    get_photo_locations_from_design(SVGFILE,&photo_locations);

    GError *error = NULL;
    RsvgHandle *handle;

    RsvgDimensionData dim;
    double width, height;
    cairo_surface_t *design_surface;
    cairo_t *design_ctx;
    cairo_status_t status;

    unsigned int img_width,img_height;
    unsigned char *img_data;
    load_png_image("../logo_whitebg.png",&img_width,&img_height,&img_data);
    rsvg_set_default_dpi (300.0);
    handle = rsvg_handle_new_from_file (SVGFILE,&error);
    rsvg_handle_get_dimensions (handle, &dim);
    width = dim.width;
    height = dim.height;

    design_surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
    design_ctx = cairo_create(design_surface);
    rsvg_handle_render_cairo(handle, design_ctx);
    status = cairo_status (design_ctx);
    if (status)
    {
        printf("cairo_status!\n");
        printf ("%s\n",cairo_status_to_string (status));
        return 1;
    }
    for(int i = 0; i < photo_locations.total_photos;i++){
        paint_photo_on_design(img_width,img_height,img_data,&photo_locations.photo_location[i],design_ctx);
    }

    cairo_surface_write_to_png (design_surface, "print_me.png");

    cairo_destroy (design_ctx);
    cairo_surface_destroy (design_surface);

    free_photo_locations(&photo_locations);


    return 0;
}
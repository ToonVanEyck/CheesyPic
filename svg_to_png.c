#include <librsvg-2.0/librsvg/rsvg.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
//#include <glib/gprintf.h>

#include "parse_svg.h"
#include "shared_memory.h"
#include "lodepng.h"


#define SVGFILE "../2x6 Ruben Dani.svg"

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
    cairo_surface_t *design_surface, *photo_surface;
    cairo_t *design_ctx, *photo_ctx;
    cairo_status_t status;
    int mode = 1;
    char * memblock;
    size_t size;

    unsigned int img_width,img_height;
    unsigned char *img_data;
    load_png_image("../logo_whitebg.png",&img_width,&img_height,&img_data);
    printf("phot %dx%d\n",img_width,img_height);
    rsvg_set_default_dpi (300.0);
    handle = rsvg_handle_new_from_file (SVGFILE,&error);
    rsvg_handle_get_dimensions (handle, &dim);
    width = dim.width;
    height = dim.height;

    printf("svg is %fx%f\n",width,height);
    photo_surface = cairo_image_surface_create_for_data (img_data,CAIRO_FORMAT_RGB24,img_width,img_height,img_width*4);
    photo_ctx = cairo_create(photo_surface);
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

    cairo_matrix_t curr_matrix,result_matrix,transform_matrix;
    cairo_matrix_init_identity (&curr_matrix);
    cairo_matrix_init_identity (&transform_matrix);

    cairo_matrix_translate(&curr_matrix,50.499943,-25.000265);
    cairo_matrix_scale(&curr_matrix,608.0/(double)img_width,456.0/(double)img_height);

    transform_matrix.xx = -0.89970838;
    transform_matrix.yx = 0.18243401;
    transform_matrix.xy = -0.18327944;
    transform_matrix.yy = -0.86352774;
    transform_matrix.x0 = 1355.1498;
    transform_matrix.y0 = 1029.9464;

    cairo_matrix_multiply (&result_matrix,&curr_matrix,&transform_matrix);
    memcpy(&curr_matrix,&result_matrix,sizeof(cairo_matrix_t));
    cairo_matrix_init_identity(&transform_matrix);


    cairo_matrix_translate(&transform_matrix,708.76158,980.34695);
    cairo_matrix_rotate(&transform_matrix,177.3573*(G_PI/180));
    cairo_matrix_translate(&transform_matrix,-708.76158,-980.34695);

    cairo_matrix_multiply (&result_matrix,&curr_matrix,&transform_matrix);
    memcpy(&curr_matrix,&result_matrix,sizeof(cairo_matrix_t));
    cairo_matrix_init_identity(&transform_matrix);

        printf("matrix(%f,%f,%f,%f,%f,%f)\n",
        curr_matrix.xx,
        curr_matrix.yx,
        curr_matrix.xy,
        curr_matrix.yy,
        curr_matrix.x0,
        curr_matrix.y0);


    cairo_set_matrix(design_ctx,&curr_matrix);
    cairo_set_source_surface(design_ctx,photo_surface,00,00);
    cairo_paint(design_ctx);

    // unsigned char * cairo_image_surface_get_data (cairo_surface_t *surface);
    cairo_surface_write_to_png (design_surface, "print_me.png");

    // int raw_width = cairo_image_surface_get_width (surface);
    // int raw_height = cairo_image_surface_get_height (surface);
    // int row_byte_size = cairo_image_surface_get_stride (surface);
    // printf("\nWIDTH: %d, HEIGHT: %d, row_bytes=%d\n", raw_width, raw_height, row_byte_size);

    // unsigned char * raw_buffer = cairo_image_surface_get_data(surface);

    // FILE * pFile;
    // pFile = fopen ("myfile.bin", "wb");
    // fwrite (raw_buffer , sizeof(char), raw_height*row_byte_size, pFile);
    // fclose (pFile);

    cairo_destroy (design_ctx);
    cairo_surface_destroy (design_surface);

    cairo_destroy (photo_ctx);
    cairo_surface_destroy (photo_surface);

    free_photo_locations(&photo_locations);

    return 0;
}
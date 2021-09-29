#include "design.h"

int main(char argc, char **argv){
    if(argc != 3){
        printf("example requires design file and a .jpg image as an argument!\n");
        printf("usage: %s my_design.design.svg image.jpg\n",argv[0]);
        return 1;
    }
    design_t design;
    load_design_from_file(&design, argv[1]);

    FILE *img;
    img = fopen(argv[2], "rb");
    if( img == NULL )
    {
        printf("Failed to open image: %s\n",argv[2]);
        return 1;
    }

    fseek(img, 0, SEEK_END);
    size_t size = ftell(img);

    jpg_photo_t *jpg_photo = malloc(design.total_photos * sizeof(jpg_photo_t));
    for(int i=0; i<design.total_photos; i++){
        fseek(img, 0, SEEK_SET);
        jpg_photo[i].size = size;
        jpg_photo[i].data = malloc(size); // allocate enough memory.
        memset(jpg_photo[i].data,0,size);
        fread(jpg_photo[i].data,size,1,img); // read in the file
    }
    fclose(img);

    // render_design(&design,jpg_photo);
    render_design(&design,jpg_photo);

    for(int i=0; i<design.total_photos; i++){
        free(jpg_photo[i].data);
    }
    free(jpg_photo);

    free_design(&design);
}
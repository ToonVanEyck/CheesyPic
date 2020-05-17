#include "logic.h"

char* get_state_name(logic_state_t i)
{
    static char *names[] = {
        "\"idle\"",
        "\"triggred\"",
        "\"countdown_3\"",
        "\"countdown_2\"",
        "\"countdown_1\"",
        "\"capture\"",
        "\"preview\"",
        "\"procces\"",
        "\"print\""};
    if(i >= 0 && i < sizeof(names)/sizeof(char*)){
        return names[i];
    }
    return "UNDEFINED";
}

int init_logic(photobooth_config_t *pbc)
{
    memset(pbc,0,sizeof(photobooth_config_t));

    read_config(pbc);
}

int load_png_image(overlay_t *overlay)
{
    unsigned error = 0;

    unsigned char* png = 0;
    size_t pngsize;

    error = lodepng_load_file(&png, &pngsize, overlay->path);
    if(!error) error = lodepng_decode32(&overlay->data,
                                        &overlay->width, 
                                        &overlay->height, 
                                        png, pngsize);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    free(png);
    return error;
}

void free_config(photobooth_config_t *pbc)
{
    free(pbc->countdown.data.image.cd_3.data);
    pbc->countdown.data.image.cd_3.data = NULL;
    free(pbc->countdown.data.image.cd_2.data);
    pbc->countdown.data.image.cd_2.data = NULL;
    free(pbc->countdown.data.image.cd_2.data);
    pbc->countdown.data.image.cd_2.data = NULL;
    free(pbc->idle.data);
    pbc->idle.data = NULL;
    free(pbc->print.data);
    pbc->print.data = NULL;
    free(pbc->smile.data);
    pbc->smile.data = NULL;
}

int read_config(photobooth_config_t *pbc)
{
    pbc->countdown.type = image_png;
    pbc->countdown.data.image.delay_ms = 1000;
    pbc->countdown.data.image.cd_3.path = "../overlays/3.png";
    pbc->countdown.data.image.cd_2.path = "../overlays/2.png";
    pbc->countdown.data.image.cd_1.path = "../overlays/1.png";
    pbc->idle.path = "../overlays/push.png";
    pbc->print.path = "../overlays/printing.png";
    pbc->smile.path = "../overlays/accept.png";

    //load png resources
    if(load_png_image(&pbc->countdown.data.image.cd_3)) return -1;
    if(load_png_image(&pbc->countdown.data.image.cd_2)) return -1;
    if(load_png_image(&pbc->countdown.data.image.cd_1)) return -1;
    if(load_png_image(&pbc->idle)) return -1;
    if(load_png_image(&pbc->print)) return -1;
    if(load_png_image(&pbc->smile)) return -1;
}

void set_image_overlay(overlay_bufer_t *dest, overlay_t *src)
{
    dest->height = src->height;
    dest->width = src->width;
    memcpy(&dest->raw_data,src->data,src->width*src->height*4);
}

void run_logic(shared_memory_t *shared_memory,photobooth_config_t *pbc)
{
    static logic_state_t prev_logic_state = -1;
    if(prev_logic_state != shared_memory->logic_state){
        prev_logic_state = shared_memory->logic_state;
        printf("%sEntered %s state!\n",PL,get_state_name(prev_logic_state));
        switch (shared_memory->logic_state){
            case log_idle:
                set_image_overlay(&shared_memory->overlay_buffer,&pbc->idle);
                break;
            case log_countdown_3:
                set_image_overlay(&shared_memory->overlay_buffer,&pbc->countdown.data.image.cd_3);
                break;
            case log_countdown_2:
                set_image_overlay(&shared_memory->overlay_buffer,&pbc->countdown.data.image.cd_2);
                break;
            case log_countdown_1:
                set_image_overlay(&shared_memory->overlay_buffer,&pbc->countdown.data.image.cd_1);
                break;

            case log_print:
                set_image_overlay(&shared_memory->overlay_buffer,&pbc->print);
                break;

            default:
                break;
        }
    }
}
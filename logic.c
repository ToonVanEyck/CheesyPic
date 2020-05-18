#include "logic.h"

static int alarm_var;

void alarm_capture(int dummy)
{
    alarm_var = 1;
}

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

int init_logic(photobooth_config_t *config, photobooth_session_t *session)
{
    signal(SIGALRM, alarm_capture);
    alarm_var = 0;
    memset(config,0,sizeof(photobooth_config_t));
    memset(session,0,sizeof(photobooth_session_t));
    read_config(config);
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

void free_config(photobooth_config_t *config)
{
    free(config->countdown.data.image.cd_3.data);
    config->countdown.data.image.cd_3.data = NULL;
    free(config->countdown.data.image.cd_2.data);
    config->countdown.data.image.cd_2.data = NULL;
    free(config->countdown.data.image.cd_2.data);
    config->countdown.data.image.cd_2.data = NULL;
    free(config->idle.data);
    config->idle.data = NULL;
    free(config->print.data);
    config->print.data = NULL;
    free(config->smile.data);
    config->smile.data = NULL;
}

int read_config(photobooth_config_t *config)
{
    config->countdown.type = image_png;
    config->countdown.data.image.cd_3.path = "../overlays/3.png";
    config->countdown.data.image.cd_2.path = "../overlays/2.png";
    config->countdown.data.image.cd_1.path = "../overlays/1.png";
    config->idle.path = "../overlays/push.png";
    config->print.path = "../overlays/printing.png";
    config->smile.path = "../overlays/accept.png";

    config->countdown.data.image.delay.it_value.tv_sec = 1;

    config->num_photos_in_design = 3;
    
    //load png resources
    if(load_png_image(&config->countdown.data.image.cd_3)) return -1;
    if(load_png_image(&config->countdown.data.image.cd_2)) return -1;
    if(load_png_image(&config->countdown.data.image.cd_1)) return -1;
    if(load_png_image(&config->idle)) return -1;
    if(load_png_image(&config->print)) return -1;
    if(load_png_image(&config->smile)) return -1;
}

void set_image_overlay(overlay_buffer_t *dest, overlay_t *src)
{
    dest->height = src->height;
    dest->width = src->width;
    memcpy(&dest->raw_data,src->data,src->width*src->height*4);
}

void run_logic(shared_memory_t *shared_memory,photobooth_config_t *config, photobooth_session_t *session)
{
    static logic_state_t prev_logic_state = -1;
    static int prev_is_active = 0;
    int init_state = (prev_logic_state != shared_memory->logic_state);

    if(prev_is_active != shared_memory->photobooth_active){
        prev_is_active = shared_memory->photobooth_active;
        printf("%sLogic is %s!\n",PL,shared_memory->photobooth_active?"active":"disabled");
    }

    if(init_state){
        prev_logic_state = shared_memory->logic_state;
        printf("%sEntered %s state!\n",PL,get_state_name(prev_logic_state));
    }


    switch (shared_memory->logic_state){
        case log_idle:          set_image_overlay(&shared_memory->overlay_buffer,&config->idle);                        break;
        case log_triggred:                                                                                              break;
        case log_countdown_3:   set_image_overlay(&shared_memory->overlay_buffer,&config->countdown.data.image.cd_3);   break;
        case log_countdown_2:   set_image_overlay(&shared_memory->overlay_buffer,&config->countdown.data.image.cd_2);   break;
        case log_countdown_1:   set_image_overlay(&shared_memory->overlay_buffer,&config->countdown.data.image.cd_1);   break;
        case log_print:         set_image_overlay(&shared_memory->overlay_buffer,&config->print);                       break;
        default:                                                                                                        break;
    }

    if(shared_memory->photobooth_active){
        switch (shared_memory->logic_state){
            case log_idle:
                if(init_state){
                }
                break;
            case log_triggred:
                if(init_state){
                    if(session->photo_counter < config->num_photos_in_design){
                        shared_memory->logic_state = log_countdown_3;
                    }
                }
                break;
            case log_countdown_3:
                if(init_state){
                    setitimer(ITIMER_REAL,&config->countdown.data.image.delay,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_countdown_2;
                    alarm_var = 0;
                }

                break;
            case log_countdown_2:
                if(init_state){
                    setitimer(ITIMER_REAL,&config->countdown.data.image.delay,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_countdown_1;
                    alarm_var = 0;
                }
                break;
            case log_countdown_1:
                if(init_state){
                    setitimer(ITIMER_REAL,&config->countdown.data.image.delay,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_print;
                    alarm_var = 0;
                }

                break;

            case log_print:
                if(init_state){
                }

                break;

            default:
                break;
        }
    }
}
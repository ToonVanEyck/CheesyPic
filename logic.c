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
        "\"decode\"",
        "\"preview\"",
        "\"procces\"",
        "\"print\""};
    if(i >= 0 && i < sizeof(names)/sizeof(char*)){
        return names[i];
    }
    return "UNDEFINED";
}

int init_logic(shared_memory_t *shared_memory, photobooth_config_t *config, photobooth_session_t *session, printer_info_t *printer_info)
{
    signal(SIGALRM, alarm_capture);

    printer_info->nuf_of_decks = 2;
    printer_info->deck = malloc(printer_info->nuf_of_decks * sizeof(deck_info_t));
    memset(printer_info->deck,0,printer_info->nuf_of_decks * sizeof(deck_info_t));

    alarm_var = 0;
    memset(config,0,sizeof(photobooth_config_t));
    memset(session,0,sizeof(photobooth_session_t));
    config->preview_mirror = &shared_memory->preview_mirror;
    config->reveal_mirror = &shared_memory->reveal_mirror;
    read_config(config);
    if(get_printer_driver_name(&config->printer_driver_name)) return 1;
    #ifndef NO_PRINT
        is_printing_finished(config->printer_driver_name,printer_info);
        if(!printer_info->connected){
            printf("Error [%s] printer connected.\n",config->printer_driver_name);
            return 1;
        } 
    #endif
    return 0;
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

void free_logic(photobooth_config_t *config, printer_info_t *printer_info)
{
    //config
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
    if(config->printer_driver_name){
        free(config->printer_driver_name);
        config->printer_driver_name = NULL;
    }
    //printer_info
    free(printer_info->deck);
    printer_info->deck = NULL;
}

int read_config(photobooth_config_t *config)
{
    config->countdown.type = image_png;
    config->countdown.data.image.cd_3.path = "../overlays/3.png";
    config->countdown.data.image.cd_2.path = "../overlays/2.png";
    config->countdown.data.image.cd_1.path = "../overlays/1.png";
    config->idle.path = "../overlays/push.png";
    config->print.path = "../overlays/printing.png";
    config->smile.path = "../overlays/smile.png";

    config->countdown.data.image.delay.it_value.tv_sec = 1;

    config->num_photos_in_design = 3;
    
    //load png resources
    if(load_png_image(&config->countdown.data.image.cd_3)) return -1;
    if(load_png_image(&config->countdown.data.image.cd_2)) return -1;
    if(load_png_image(&config->countdown.data.image.cd_1)) return -1;
    if(load_png_image(&config->idle)) return -1;
    if(load_png_image(&config->print)) return -1;
    if(load_png_image(&config->smile)) return -1;

    config->preview_time.it_value.tv_sec = 3;
    *config->preview_mirror = 1;

    *config->reveal_mirror = 1;
}

void set_image_overlay(overlay_buffer_t *dest, overlay_t *src)
{
    dest->height = src->height;
    dest->width = src->width;
    memcpy(&dest->raw_data,src->data,src->width*src->height*4);
}

void run_logic(shared_memory_t *shared_memory,photobooth_config_t *config, photobooth_session_t *session, printer_info_t *printer_info)
{
    static logic_state_t prev_logic_state = -1;
    int init_state = (prev_logic_state != shared_memory->logic_state);

    static int prev_is_active = 2;
    if(prev_is_active != shared_memory->photobooth_active){
        prev_is_active = shared_memory->photobooth_active;
        printf("%slogic active:   [%s]\n",PL,shared_memory->photobooth_active?"YES":"NO");
    }

    static int prev_preview_mirror = 2;
    if(prev_preview_mirror != shared_memory->preview_mirror){
        prev_preview_mirror = shared_memory->preview_mirror;
        printf("%smirror preview: [%s]\n",PL,shared_memory->preview_mirror?"YES":"NO");
    }

    static int prev_reveal_mirror = 2;
    if(prev_reveal_mirror != shared_memory->reveal_mirror){
        prev_reveal_mirror = shared_memory->reveal_mirror;
        printf("%smirror reveal:  [%s]\n",PL,shared_memory->reveal_mirror?"YES":"NO");
    }

    static int prev_fastmode = 2;
    if(prev_fastmode != shared_memory->fastmode){
        prev_fastmode = shared_memory->fastmode;
        printf("%sfast mode:      [%s]\n",PL,shared_memory->fastmode?"YES":"NO");        
    }

    if(init_state){
        prev_logic_state = shared_memory->logic_state;
        printf("%sEntered %s state!\n",PL,get_state_name(prev_logic_state));
    }

    // select overlays here!
    switch (shared_memory->logic_state){
        case log_idle:          set_image_overlay(&shared_memory->overlay_buffer,&config->idle);                        break;
        case log_triggred:                                                                                              break;
        case log_countdown_3:   set_image_overlay(&shared_memory->overlay_buffer,&config->countdown.data.image.cd_3);   break;
        case log_countdown_2:   set_image_overlay(&shared_memory->overlay_buffer,&config->countdown.data.image.cd_2);   break;
        case log_countdown_1:   set_image_overlay(&shared_memory->overlay_buffer,&config->countdown.data.image.cd_1);   break;
        case log_capture:       set_image_overlay(&shared_memory->overlay_buffer,&config->smile);                       break;
        case log_reveal:                                                                                                break; //the captured image is shown instead of the overlay
        case log_procces:       set_image_overlay(&shared_memory->overlay_buffer,&config->print);                       break;
        default:                                                                                                        break;
    }

    struct itimerval fast_time = {{0,0},{0,100000}};

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
                    }else{
                         shared_memory->logic_state = log_procces;
                    }
                }
                break;
            case log_countdown_3:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown.data.image.delay,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_countdown_2;
                    alarm_var = 0;
                }

                break;
            case log_countdown_2:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown.data.image.delay,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_countdown_1;
                    alarm_var = 0;
                }
                break;
            case log_countdown_1:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown.data.image.delay,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_capture;
                    alarm_var = 0;
                }

                break;
            case log_capture:
                // handled in capture thread
                if(init_state){
                    printf("%scaptured %d/%d photos.\n",PL,session->photo_counter+1,config->num_photos_in_design);
                    session->photo_counter++;
                }
                break;
            case log_decode:
                // handled in decode thread
                break;
            case log_reveal:
                // handled in decode thread
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->preview_time,NULL);
                }
                if(alarm_var && shared_memory->capture_buffer.jpeg_data == NULL){
                    shared_memory->logic_state = log_triggred;
                    alarm_var = 0;
                }
                break;

            case log_procces:
                if(init_state){
                        FILE *source, *target;
                        source = fopen("../kittens/2.jpg", "rb");
                        if( source == NULL )
                        {
                            printf("Failed to open src file...\n");
                            break;
                        }
                        target = fopen("print_me.jpg", "wb+");
                        if( target == NULL )
                        {
                            fclose(source);
                            printf("Failed to open dest file...\n");
                            break;
                        }
                        size_t n, m;
                        unsigned char buff[8192];
                        do {
                            n = fread(buff, 1, sizeof buff, source);
                            if (n) m = fwrite(buff, 1, n, target);
                            else   m = 0;
                        } while ((n > 0) && (n == m));
                        if (m) perror("copy");
                        printf("File copied successfully.\n");
                        fclose(source);
                        fclose(target);
                }
                #ifdef NO_PRINT
                    shared_memory->logic_state = log_idle;
                #else
                    shared_memory->logic_state = log_print;
                #endif
                break;

            case log_print:
                if(init_state){
                    print_file("print_me.jpg");
                }
                if(is_printing_finished(config->printer_driver_name,printer_info)) shared_memory->logic_state = log_idle;
                break;

            default:
                break;
        }
    }
}
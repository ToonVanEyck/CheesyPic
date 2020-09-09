#include "logic.h"

static int alarm_var;

void alarm_capture(int dummy)
{
    alarm_var = 1;
}

static unsigned long writeJpg(char *name, const char *data, unsigned long size){
    FILE *file;
    file = fopen(name, "wb+");
    if (!file)
    {
        LOG("Unable to open file %s\n", name);
        return 1;
    }
    
    fwrite(data, size, 1, file);
    fclose(file);
    return size;
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
        "\"reveal\"",
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
    session->capture_data = malloc(sizeof(char *)*config->design.total_photos);
    if(session->capture_data == NULL){
        LOG("Error couldn't allocate memory for session\n");
        return 1;
    }

    #ifdef NO_PRINT
        //nothing
    #else
        if(get_printer_driver_name(&config->printer_driver_name)){
        LOG("No default printer...\n");
        return 1;
        }
        is_printing_finished(config->printer_driver_name,printer_info);
        if(!printer_info->connected){
            LOG("Error [%s] printer not connected.\n",config->printer_driver_name);
        } 
        config->printer_active = 1;
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
    if(error) LOG("error %u: %s\n", error, lodepng_error_text(error));
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
    free_design(&config->design);
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

    //config->num_photos_in_design = 3;
    
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

    //output parameters
    config->photo_output_directory = "/home/toon/Pictures/Cheesypic/Default";
    config->photo_output_name = "cheesypic_default";

    load_design_from_file(&config->design, "../design_template.svg");
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
        LOG("logic active:   [%s]\n",shared_memory->photobooth_active?"YES":"NO");
    }

    static int prev_preview_mirror = 2;
    if(prev_preview_mirror != shared_memory->preview_mirror){
        prev_preview_mirror = shared_memory->preview_mirror;
        LOG("mirror preview: [%s]\n",shared_memory->preview_mirror?"YES":"NO");
    }

    static int prev_reveal_mirror = 2;
    if(prev_reveal_mirror != shared_memory->reveal_mirror){
        prev_reveal_mirror = shared_memory->reveal_mirror;
        LOG("mirror reveal:  [%s]\n",shared_memory->reveal_mirror?"YES":"NO");
    }

    static int prev_fastmode = 2;
    if(prev_fastmode != shared_memory->fastmode){
        prev_fastmode = shared_memory->fastmode;
        LOG("fast mode:      [%s]\n",shared_memory->fastmode?"YES":"NO");        
    }

    static int prev_printing_wanted = 2;
    if(prev_printing_wanted != shared_memory->printing_wanted){
        prev_printing_wanted = shared_memory->printing_wanted;
        if(shared_memory->printing_wanted){
            is_printing_finished(config->printer_driver_name,printer_info);
            if(printer_info->connected){
                config->printer_active = 1;
            }else{
                config->printer_active = 0;
            }
        } else {
            config->printer_active = 0;
        }
        LOG("printing wanted:[%s]\n",shared_memory->printing_wanted?"YES":"NO");     
        if(shared_memory->printing_wanted) LOG("printing active:[%s]\n",config->printer_active?"YES":"NO");    
    }

    if(init_state){
        prev_logic_state = shared_memory->logic_state;
        LOG("Entered %s state!\n",get_state_name(prev_logic_state));
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
        sem_post(&shared_memory->sem_render);
    }
    struct itimerval fast_time = {{0,0},{0,100000}};

    if(shared_memory->photobooth_active){
        switch (shared_memory->logic_state){
            case log_idle:
                if(init_state){
                    session->photo_counter=0;
                    //free all session memory here?
                }
                break;
            case log_triggred:
                if(init_state){
                    if(session->photo_counter < config->design.total_photos){
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
                    //for(int i = 0;i<NUM_JPEG_BUFFERS;i++)shared_memory->preview_buffer[0].pre_state = pre_render;
                    LOG("captured %d/%d photos.\n",session->photo_counter+1,config->design.total_photos);
                    session->photo_counter++;
                }
                break;
            case log_decode:
                if(init_state){
                }
                // handled in decode thread
                break;
            case log_reveal:
                // handled in decode thread
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->preview_time,NULL);
                    // save image
                    char capture_path[128]={0};
                    time_t now;
                    struct tm ts;
                    time(&now);
                    ts = *localtime(&now);
                    sprintf(capture_path,"%s/%s",config->photo_output_directory,config->photo_output_name);
                    strftime(&capture_path[strlen(capture_path)], sizeof(capture_path), "_%y%m%d_%H%M%S", &ts);
                    sprintf(&capture_path[strlen(capture_path)],"_%d.jpg",session->photo_counter);
                    #ifndef NO_SAVE
                        writeJpg(capture_path,(const char *)shared_memory->capture_buffer.jpeg_buffer,shared_memory->capture_buffer.size);
                    #endif
                    // encode base64 image
                    /* set up a destination buffer large enough to hold the encoded data */
                    int encoded_size = 32 + 2*shared_memory->capture_buffer.size;
                    LOG("original size %ld --> base 64size %d\n",shared_memory->capture_buffer.size,encoded_size);
                    session->capture_data[session->photo_counter-1] = malloc(encoded_size);
                    if(session->capture_data[session->photo_counter-1] == NULL){
                        LOG("failed to allocate memory for capture copy");
                        exit(1);
                    }
                    strcpy(session->capture_data[session->photo_counter-1],"data:image/jpeg;base64,");
                    /* keep track of our encoded position */
                    char* c = session->capture_data[session->photo_counter-1]+23;
                    /* store the number of bytes encoded by a single call */
                    int cnt = 0;
                    /* we need an encoder state */
                    base64_encodestate s;

                    /*---------- START ENCODING ----------*/
                    /* initialise the encoder state */
                    base64_init_encodestate(&s);
                    /* gather data from the input and send it to the output */
                    cnt = base64_encode_block(shared_memory->capture_buffer.jpeg_buffer, shared_memory->capture_buffer.size, c, &s);
                    c += cnt;
                    /* since we have encoded the entire input string, we know that 
                    there is no more input data; finalise the encoding */
                    cnt = base64_encode_blockend(c, &s);
                    c += cnt;
                    /*---------- STOP ENCODING  ----------*/

                    // /* we want to print the encoded data, so null-terminate it: */
                    *c = 0;

                    // FILE *fp = fopen("base64.txt", "w");
                    // fLOG(fp,"%s",session->capture_data[session->photo_counter-1]);
                    // fclose(fp);

                    shared_memory->capture_buffer.jpeg_copied = 1;
                }
                if(alarm_var && shared_memory->capture_buffer.jpeg_copied == 0){
                    shared_memory->logic_state = log_triggred;
                    alarm_var = 0;
                }
                break;

            case log_procces:
                if(init_state){
                        render_design(&config->design,session->capture_data);
                        for(int i = 0; i<config->design.total_photos;i++){
                            free(session->capture_data[i]);
                        }

                        // FILE *source, *target;
                        // source = fopen("../kittens/2.jpg", "rb");
                        // if( source == NULL )
                        // {
                        //     LOG("Failed to open src file...\n");
                        //     break;
                        // }
                        // target = fopen("print_me.jpg", "wb+");
                        // if( target == NULL )
                        // {
                        //     fclose(source);
                        //     LOG("Failed to open dest file...\n");
                        //     break;
                        // }
                        // size_t n, m;
                        // unsigned char buff[8192];
                        // do {
                        //     n = fread(buff, 1, sizeof buff, source);
                        //     if (n) m = fwrite(buff, 1, n, target);
                        //     else   m = 0;
                        // } while ((n > 0) && (n == m));
                        // if (m) perror("copy");
                        // LOG("File copied successfully.\n");
                        // fclose(source);
                        // fclose(target);
                }
                    if(config->printer_active){
                        shared_memory->logic_state = log_print;
                    }else{
                        shared_memory->logic_state = log_idle;
                    }
                break;

            case log_print:
                if(init_state){
                    print_file("print_me.png");
                }
                if(is_printing_finished(config->printer_driver_name,printer_info)) shared_memory->logic_state = log_idle;
                break;

            default:
                break;
        }
    }
}
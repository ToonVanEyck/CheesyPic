#include "logic.h"

static int alarm_var;
static void *thread_data[2] = {NULL,NULL};

void alarm_capture(int dummy)
{
    alarm_var = 1;
}

static int logicRunning = 1;
void stop_logic_thread(int dummy)
{
    logicRunning = 0;
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

pthread_t photostrip_render_thread;
void *photostrip_render(void* data) {
    LOG("started rendering the design\n");
    design_t *design = (design_t *)((void **)data)[0]; 
    jpg_photo_t *jpg_capture = (jpg_photo_t *)((void **)data)[1]; 
    render_design(design,jpg_capture);
    for(int i = 0; i<design->total_photos;i++){
        free(jpg_capture[i].data);
        jpg_capture[i].size = 0;
    }
    return NULL;
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
        "\"capture_failed\"",
        "\"reveal\"",
        "\"procces\"",
        "\"print\""};
    if(i >= 0 && i < sizeof(names)/sizeof(char*)){
        return names[i];
    }
    return "UNDEFINED";
}

void init_logic()
{
    signal(SIGALRM, alarm_capture);
    signal(SIGINT, stop_logic_thread);
    alarm_var = 0;
}

void set_image_overlay(overlay_buffer_t *dest, overlay_t *src)
{
    dest->height = src->height;
    dest->width = src->width;
    memcpy(&dest->raw_data,src->data,src->width*src->height*4);
}

int run_logic(shared_memory_t *shared_memory,config_t *config, session_t *session, printer_info_t *printer_info)
{
    static logic_state_t prev_logic_state = -1;
    int init_state = (prev_logic_state != shared_memory->logic_state);

    static int prev_is_active = 2;
    if(prev_is_active != shared_memory->photobooth_active){
        prev_is_active = shared_memory->photobooth_active;
        init_state = (shared_memory->photobooth_active == 1);
        LOG("logic active:   [%s]\n",shared_memory->photobooth_active?"YES":"NO");
    }

    static int prev_mirror_liveview = 2;
    if(prev_mirror_liveview != shared_memory->mirror_liveview){
        prev_mirror_liveview = shared_memory->mirror_liveview;
        LOG("mirror preview: [%s]\n",shared_memory->mirror_liveview?"YES":"NO");
    }

    static int prev_mirror_preview = 2;
    if(prev_mirror_preview != shared_memory->mirror_preview){
        prev_mirror_preview = shared_memory->mirror_preview;
        LOG("mirror reveal:  [%s]\n",shared_memory->mirror_preview?"YES":"NO");
    }

    static int prev_fastmode = 2;
    if(prev_fastmode != shared_memory->fastmode){
        prev_fastmode = shared_memory->fastmode;
        LOG("fast mode:      [%s]\n",shared_memory->fastmode?"YES":"NO");        
    }

    if(shared_memory->toggle_printer){
        shared_memory->toggle_printer = 0;
        is_printing_finished(config->printer_driver_name,printer_info);
        if(printer_info->connected){
            config->printing_enabled ^= 1;
        }else{
            config->printing_enabled = 0;
        }
        LOG("printer connected:[%s]\n",printer_info->connected?"YES":"NO");     
        if(printer_info->connected) LOG("printing enabled:[%s]\n",config->printing_enabled?"YES":"NO");    
    }

    if(init_state){
        prev_logic_state = shared_memory->logic_state;
        LOG("Entered %s state!\n",get_state_name(prev_logic_state));
        // select overlays here!
        switch (shared_memory->logic_state){
            case log_idle:          set_image_overlay(&shared_memory->overlay_buffer,&config->theme.push);   break;
            case log_triggred:                                                                               break;
            case log_countdown_3:   set_image_overlay(&shared_memory->overlay_buffer,&config->theme.cd_3);   break;
            case log_countdown_2:   set_image_overlay(&shared_memory->overlay_buffer,&config->theme.cd_2);   break;
            case log_countdown_1:   set_image_overlay(&shared_memory->overlay_buffer,&config->theme.cd_1);   break;
            case log_capture:       set_image_overlay(&shared_memory->overlay_buffer,&config->theme.smile);  break;
            case log_capture_failed:set_image_overlay(&shared_memory->overlay_buffer,&config->theme.fail);   break;
            case log_reveal:                                                                                 break; //the captured image is shown instead of the overlay
            case log_procces:                                                                                break;
            case log_print:         set_image_overlay(&shared_memory->overlay_buffer,&config->theme.print);  break;
            default:                                                                                         break;
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
                    shared_memory->logic_state = log_countdown_3;
                }
                break;
            case log_countdown_3:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown_time,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_countdown_2;
                    alarm_var = 0;
                }

                break;
            case log_countdown_2:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown_time,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_countdown_1;
                    alarm_var = 0;
                }
                break;
            case log_countdown_1:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown_time,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_capture;
                    alarm_var = 0;
                }
                break;
            case log_capture:
                // handled in capture thread
                if(init_state){
                }
                break;
            case log_capture_failed:
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->countdown_time,NULL);
                }
                if(alarm_var){
                    shared_memory->logic_state = log_triggred;
                    alarm_var = 0;
                }
                break;
            case log_decode:
                if(init_state){
                    session->photo_counter++;
                    LOG("captured %d/%d photos.\n",session->photo_counter,config->design.total_photos);
                }
                // handled in decode thread
                break;
            case log_reveal:
                // handled in decode thread
                if(init_state){
                    setitimer(ITIMER_REAL,shared_memory->fastmode?&fast_time:&config->preview_time,NULL);
                    // save image
                    char capture_path[256]={0};
                    time_t now;
                    struct tm ts;
                    time(&now);
                    ts = *localtime(&now);
                    strftime(&capture_path[strlen(capture_path)], sizeof(capture_path), "%y%m%d_%H%M%S", &ts);
                    sprintf(&capture_path[strlen(capture_path)],"_%d.jpg",session->photo_counter);
                    if(config->save_photos){
                        writeJpg(capture_path,(const char *)shared_memory->capture_buffer.jpeg_buffer,shared_memory->capture_buffer.size);
                    }
                    session->jpg_capture[session->photo_counter-1].size = shared_memory->capture_buffer.size;
                    session->jpg_capture[session->photo_counter-1].data = malloc(session->jpg_capture[session->photo_counter-1].size);
                    memcpy(session->jpg_capture[session->photo_counter-1].data,shared_memory->capture_buffer.jpeg_buffer,session->jpg_capture[session->photo_counter-1].size);
                    shared_memory->capture_buffer.jpeg_copied = 1;
                    photostrip_render_thread = 0;
                    
                }
                if(shared_memory->capture_buffer.jpeg_copied == 0){
                    if(photostrip_render_thread == 0 && session->photo_counter == config->design.total_photos && config->printing_enabled){
                            thread_data[0] = &config->design;
                            thread_data[1] = session->jpg_capture;
                            pthread_create(&photostrip_render_thread, NULL,photostrip_render,thread_data);
                        }
                    if(alarm_var /*&& shared_memory->capture_buffer.jpeg_copied == 0*/){
                        if(session->photo_counter != config->design.total_photos){
                            shared_memory->logic_state = log_triggred;
                        }else if(config->printing_enabled){
                            shared_memory->logic_state = log_print;
                        }else{
                            shared_memory->logic_state = log_idle;
                        }
                        alarm_var = 0;
                    }
                }
                break;

            case log_procces:
                if(init_state){
                    render_design(&config->design,session->jpg_capture);
                    for(int i = 0; i<config->design.total_photos;i++){
                        free(session->jpg_capture[i].data);
                        session->jpg_capture[i].size = 0;
                    }
                }
                if(alarm_var){
                    if(config->printing_enabled){
                        shared_memory->logic_state = log_print;
                    }else{
                        shared_memory->logic_state = log_idle;
                    }
                    alarm_var = 0;
                }
                break;
            case log_print:
                if(init_state){
                    void *retval;
                    pthread_join(photostrip_render_thread,&retval);
                    print_file("print_me.png");
                }
                if(is_printing_finished(config->printer_driver_name,printer_info)) shared_memory->logic_state = log_idle;
                break;

            default:
                break;
        }
    }
    return logicRunning;
}
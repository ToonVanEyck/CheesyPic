#define _GNU_SOURCE

#include "logic.h"
#include "render_thread.h"
#include "decode_thread.h"
#include "capture_thread.h"
#include "shared_memory.h"
#include "config.h"

int main(int argc, char *argv[])
{
    int exit_code = EXIT_SUCCESS;
    // Init shared memory
    LOG("allocating %ld bytes of shared memory.\n",sizeof(shared_memory_t));
    shared_memory_t *shared_memory = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Init semaphores
    sem_init(&shared_memory->sem_decode,0,0);
    sem_init(&shared_memory->sem_render,1,0);
    sem_init(&shared_memory->sem_logic,1,0);
    shared_memory->photobooth_active = 1;
    shared_memory->fastmode = 0;
    shared_memory->exit_slow = 0;
    #ifdef FAST_MODE
        shared_memory->fastmode = 1;
    #endif
    // Read configuration file
    config_t config;
    if(read_config(&config)){
        LOG("Failed to parse config file!\n");
        exit_code = EXIT_FAILURE; 
        goto cleanup_config;
    }
    // Setup shared memory data
    shared_memory->mirror_liveview = config.mirror_liveview;
    shared_memory->mirror_preview = config.mirror_preview;
    shared_memory->windowless_mode = config.windowless_mode;
    // Start addons
    if(config.addon_script != NULL){
        LOG("Starting addon: %s\n",config.addon_script);
            pid_t addon_pid = fork();
            if(!addon_pid){
                char* pid_str;
                asprintf(&pid_str,"%d",getppid());
                char *args[] = {config.addon_script,pid_str,NULL};
                execv(args[0],args);
                exit(EXIT_FAILURE);
            }
    }
    // Init printer data
    printer_info_t printer_info;
    printer_info.nuf_of_decks = 2;
    printer_info.deck = malloc(printer_info.nuf_of_decks * sizeof(deck_info_t));
    memset(printer_info.deck,0,printer_info.nuf_of_decks * sizeof(deck_info_t));
    if(get_printer_name(&config.printer_name) || get_printer_driver_name(&config.printer_driver_name)){
        LOG("No default printer...\n");
        exit_code = EXIT_FAILURE; 
        goto cleanup_printer;
    }
    init_printer(config.printer_name);
    is_printing_finished(config.printer_driver_name,&printer_info);
    if(!printer_info.connected){
        LOG("Error [%s] printer not connected.\n",config.printer_driver_name);
        config.printing_enabled = 0;
    }
    // Init session data
    session_t session;
    memset(&session,0,sizeof(session));
    session.jpg_capture = malloc(config.design.total_photos * sizeof(jpg_photo_t));
    if(session.jpg_capture == NULL){
        LOG("Error couldn't allocate memory for session\n");
        exit_code = EXIT_FAILURE; 
        goto cleanup_session;
    }
    init_logic();
    // start threads
    pid_t capture_pid = fork();
    LOG("capture_pid %d -- %d\n",capture_pid,getpid());
    if(!capture_pid){
        start_capture_thread(shared_memory);
        exit(EXIT_FAILURE);
    }
    pid_t render_pid = fork();
    LOG("render_pid %d -- %d\n",render_pid,getpid());
    if(!render_pid){
        start_render_thread(shared_memory);
        exit(EXIT_FAILURE);
    }
    // run logic and wait for threads / processes to finish
    int status = 0;
    // int c = 0;
    
    int logic_running = 1;
    const struct timespec sem_timespec = {0,250000};
    while(logic_running && capture_pid && render_pid){
        sem_timedwait(&shared_memory->sem_logic,&sem_timespec);
        // check if a proccess has died.
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == getpid()){
            LOG("This thread got a signal\n");
        }
        if(pid == capture_pid) capture_pid = 0; 
        if(pid == render_pid) render_pid = 0; 
        // execute photobooth logic
        logic_running = run_logic(shared_memory,&config, &session, &printer_info);
    }
    //cleanup code
    sleep(1);
    LOG("Killing all threads\n");
    //kill other threads
    if(capture_pid)kill(capture_pid,SIGINT);
    if(render_pid)kill(render_pid,SIGINT);
    while(capture_pid || render_pid){
        sleep(1);
        if(render_pid) sem_post(&shared_memory->sem_render);
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == capture_pid) capture_pid = 0; 
        if(pid == render_pid) render_pid = 0; 
    }
cleanup_session:
    if(session.jpg_capture != NULL) free(session.jpg_capture);
cleanup_printer:
    if(printer_info.deck != NULL) free(printer_info.deck);
cleanup_config:
    free_config(&config);
cleanup_sems:
    //destroy semaphores
    sem_destroy(&shared_memory->sem_decode);
    sem_destroy(&shared_memory->sem_render);
    sem_destroy(&shared_memory->sem_logic);

    if(shared_memory->exit_slow)sleep(20);
    exit(exit_code);
}
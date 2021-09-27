#include "logic.h"
#include "render_thread.h"
#include "decode_thread.h"
#include "capture_thread.h"
#include "shared_memory.h"

int main(int argc, char *argv[])
{
    int exit_code = EXIT_SUCCESS;
    #ifdef NO_CAM
        LOG("Not using camera!\n");
    #endif
    // check requirments

    if(argc != 2){
        LOG("ERROR: Please supply a design directory.\n");
        exit(EXIT_FAILURE);
    }
    char design_path[512]={0};
    if(get_latest_design(argv[1],design_path)) exit(EXIT_FAILURE);
    char theme_path[512]={0};
    if(get_latest_theme(argv[1],theme_path))   exit(EXIT_FAILURE);
    // setup ipc
    LOG("allocating %ld bytes of shared memory.\n",sizeof(shared_memory_t));
    shared_memory_t *shared_memory = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // init semaphores
    sem_init(&shared_memory->sem_decode,0,0);
    sem_init(&shared_memory->sem_render,1,0);
    sem_init(&shared_memory->sem_logic,1,0);
    shared_memory->photobooth_active = 1;
    shared_memory->fastmode = 0;
    shared_memory->exit_slow = 0;
    #ifdef FAST_MODE
        shared_memory->fastmode = 1;
    #endif
    // initialise printer data
    printer_info_t printer_info;
    photobooth_config_t config;
    photobooth_session_t session;
    if(init_logic(shared_memory, &config, &session, &printer_info, design_path, theme_path)){
        exit_code = EXIT_FAILURE; 
        goto cleanup;
    }
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
    int c = 0;
    
    const struct timespec sem_timespec = {0,250000};
    while(capture_pid && render_pid){
        sem_timedwait(&shared_memory->sem_logic,&sem_timespec);
        // check if a proccess has died.
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == capture_pid) capture_pid = 0; 
        if(pid == render_pid) render_pid = 0; 
        // execute photobooth logic
        run_logic(shared_memory,&config, &session, &printer_info);
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
cleanup:
    //destroy semaphores
    sem_destroy(&shared_memory->sem_decode);
    sem_destroy(&shared_memory->sem_render);
    sem_destroy(&shared_memory->sem_logic);

    free_logic((void*)&config, &printer_info);

    if(shared_memory->exit_slow)sleep(20);
    exit(exit_code);
}
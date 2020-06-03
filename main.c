#include "render_thread.h"
#include "decode_thread.h"
#include "capture_thread.h"
#include "logic.h"
#include "shared_memory.h"

int main(int argc, char *argv[])
{
    #ifdef NO_CAM
        printf("Not using camera!\n");
    #endif
    // check requirments

    // setup ipc
    printf("allocating %ld bytes of shared memory.\n",sizeof(shared_memory_t));
    shared_memory_t *shared_memory = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // init semaphores
    sem_init(&shared_memory->sem_decode,0,0);
    sem_init(&shared_memory->sem_render,1,0);
    sem_init(&shared_memory->sem_logic,1,0);
    shared_memory->photobooth_active = 1;
    shared_memory->fastmode = 0;
    #ifdef FAST_MODE
        shared_memory->fastmode = 1;
    #endif
    photobooth_config_t config;
    photobooth_session_t session;
    init_logic(shared_memory, &config, &session);
    // start threads
    pid_t capture_pid = fork();
    printf("capture_pid %d -- %d\n",capture_pid,getpid());
    if(!capture_pid){
        start_capture_thread(shared_memory);
        exit(0);
    }
    pid_t render_pid = fork();
    printf("render_pid %d -- %d\n",render_pid,getpid());
    if(!render_pid){
        start_render_thread(shared_memory);
        return 0;
    }
    // run logic and wait for threads / processes to finish
    int status = 0;
    int c = 0;
    
    const struct timespec sem_timespec = {0,100000};
    while(capture_pid && render_pid){
        sem_timedwait(&shared_memory->sem_logic,&sem_timespec);
        // check if a proccess has died.
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == capture_pid) capture_pid = 0; 
        if(pid == render_pid) render_pid = 0; 
        // execute photobooth logic
        run_logic(shared_memory,&config, &session);
    }
    //cleanup code
    sleep(1);
    printf("\033[0mKilling all threads\n");
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
    //destroy semaphores
    sem_destroy(&shared_memory->sem_decode);
    sem_destroy(&shared_memory->sem_render);
    sem_destroy(&shared_memory->sem_logic);

    free_config((void*)&config);
    exit(0);
}
#include "render_thread.h"
#include "decode_thread.h"
#include "capture_thread.h"
#include "logic.h"
#include "shared_memory.h"
#include "lodepng.h"

int main(int argc, char *argv[])
{
    #ifdef NO_CAM
        printf("Not using camera!\n");
    #endif


    // read settings

    // check requirments

    // setup ipc
    printf("allocating %ld bytes of shared memory.\n",sizeof(shared_memory_t));
    shared_memory_t *shared_memory = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // init semaphores
    sem_init(&shared_memory->sem_decode,0,0);
    sem_init(&shared_memory->sem_render,1,0);
    sem_init(&shared_memory->sem_logic,1,0);
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

    //todo remove this
    unsigned error;
    unsigned char* image = 0;
    unsigned width, height;
    unsigned char* png = 0;
    size_t pngsize;

    error = lodepng_load_file(&png, &pngsize, "push_or_scan.png");
    if(!error) error = lodepng_decode32(&image,
                                        &shared_memory->overlay_buffer.width, 
                                        &shared_memory->overlay_buffer.height, 
                                        png, pngsize);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    free(png);
    memcpy(shared_memory->overlay_buffer.raw_data,image,shared_memory->overlay_buffer.width*shared_memory->overlay_buffer.height*4);
    free(image);

    while(capture_pid && render_pid){
        if(sem_timedwait(&shared_memory->sem_logic,&sem_timespec)==0){ // this is messing up the cleanup code ...
            // check if a proccess has died.
            pid_t pid = waitpid(-1,&status,WNOHANG);
            if(pid == capture_pid) capture_pid = 0; 
            if(pid == render_pid) render_pid = 0; 
            // execute photobooth logic
            run_logic(shared_memory);
        }
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

    exit(0);
}
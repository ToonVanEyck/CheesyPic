#include "render_thread.h"
#include "decode_thread.h"
#include "capture_thread.h"
#include "photobooth_thead.h"
#include "shared_memory.h"

int main(int argc, char *argv[])
{
    // read settings

    // check requirments

    // setup ipc
    // Our memory buffer will be readable and writable:
    void *shmem = mmap(NULL, sizeof(jpeg_buffer_t)*NUM_SHARED_BUFFERS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // start threads
    pid_t capture_pid = fork();
    if(!capture_pid){
        start_capture_thread(shmem);
        return 0;
    }
    pid_t render_pid = fork();
    if(!render_pid){
        start_render_thread(shmem);
        return 0;
    }
    // wait for thread to finish
    int status = 0;
    int c = 0;
    while(capture_pid && render_pid){
        sleep(1);
        // check if a proccess has died.
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == capture_pid) capture_pid = 0; 
        if(pid == render_pid) render_pid = 0; 
    }
    printf("\033[0mKilling all threads\n");
    //kill other threads
    if(capture_pid)kill(capture_pid,SIGINT);
    if(render_pid)kill(render_pid,SIGINT);
    while(capture_pid || render_pid){
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == capture_pid) capture_pid = 0; 
        if(pid == render_pid) render_pid = 0; 
    }
    exit(0);
}
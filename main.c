#include "render_thread.h"
#include "decode_thread.h"
#include "capture_thread.h"
#include "photobooth_thead.h"

int main(int argc, char *argv[])
{
    // read settings

    // check requirments

    // setup ipc
    // Our memory buffer will be readable and writable:
    void *shmem = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // start threads
    pid_t render_pid = fork();
    if(!render_pid){
        start_capture_thread(shmem);
    }
    // wait for thread to finish
    int status = 0;
    int c = 0;
    while(render_pid){
        if(c++ > 5){
            printf("sending SIGINT to child\n");
            kill(render_pid,SIGINT);
        }
        sleep(1);
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid == render_pid) render_pid = 0;  
        if(((char*)shmem)[0]==1){
            printf("got string: \"%s\"\n",&((char*)shmem)[1]);
            ((char*)shmem)[0]=0;
        }
    }

    // wait for thread to finish

    // kill other threads

    // exit
}
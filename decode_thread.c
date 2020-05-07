#include "decode_thread.h"

static int decodeRunning;

void *start_decode_thread(void *shared_memory)
{
    printf("%sStarted decoding thread!\n",PD);
    init_decode_thread();
    decodeRunning = 1;
    run_decode_thread(shared_memory);
    printf("%sFinished decoding thread!\n",PD);
}

void stop_decode_thread()
{
    decodeRunning = 0;
}


int init_decode_thread()
{

}

void run_decode_thread(shared_memory_t *shared_memory)
{
    while(decodeRunning){ 
        sem_wait(&shared_memory->sem_decode);
        for(int i = 0;i<NUM_JPEG_BUFFERS;i++){
            if(shared_memory->buffer[0].state == decode && shared_memory->buffer[1].state == decode)printf("%serror?\n",PD);
            if(shared_memory->buffer[i].state == decode){ // wait for buffer to fill
                int jpegSubsamp;
                tjhandle _jpegDecompressor = tjInitDecompress();
                tjDecompressHeader2(_jpegDecompressor, 
                                    (unsigned char*)shared_memory->buffer[i].compressed_data, 
                                    shared_memory->buffer[i].size, 
                                    &shared_memory->buffer[i].width, 
                                    &shared_memory->buffer[i].height, 
                                    &jpegSubsamp);
                //printf("%ssize %dx%d\n",PD,shared_memory->buffer[i].width,shared_memory->buffer[i].height);
                if(shared_memory->buffer[i].width * shared_memory->buffer[i].height <= PREVIEW_WIDTH * PREVIEW_HEIGHT){               
                    tjDecompress2(  _jpegDecompressor, 
                                    (unsigned char*)shared_memory->buffer[i].compressed_data, 
                                    shared_memory->buffer[i].size, 
                                    shared_memory->buffer[i].uncompressed_data,
                                    shared_memory->buffer[i].width, 
                                    0,
                                    shared_memory->buffer[i].height, 
                                    TJPF_RGB, TJFLAG_FASTDCT);
                                    shared_memory->buffer[i].state = render;
                                    printf("%s%d Decode complete\n",PD,i);
                }else{
                    printf("%sERROR image to large\n",PD);
                    decodeRunning = 0;
                }
                tjDestroy(_jpegDecompressor);
                break;
            }
        }
        sem_post(&shared_memory->sem_render);
    }
}

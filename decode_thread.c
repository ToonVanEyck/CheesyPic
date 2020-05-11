#include "decode_thread.h"

static int decodeRunning;

void *start_decode_thread(void *shared_memory)
{
    printf("%sStarted decoding thread!\n",PD);
    init_decode_thread(((shared_memory_t*)shared_memory)->preview_buffer);
    decodeRunning = 1;
    run_decode_thread(shared_memory);
    printf("%sFinished decoding thread!\n",PD);
}

void stop_decode_thread()
{
    decodeRunning = 0;
}


int init_decode_thread(preview_buffer_t preview_buffer[])
{
    #ifdef NO_CAM   // prefill decoded buffers
        for(int i = 0; i<PREVIEW_WIDTH*PREVIEW_HEIGHT*3;i+=3){
            preview_buffer[0].raw_data[i]=255;
        }
        preview_buffer[0].height = PREVIEW_HEIGHT;
        preview_buffer[0].width = PREVIEW_WIDTH;

        for(int i = 2; i<PREVIEW_WIDTH*PREVIEW_HEIGHT*3;i+=3){
            preview_buffer[1].raw_data[i]=255;
        }
        preview_buffer[1].height = PREVIEW_HEIGHT;
        preview_buffer[1].width = PREVIEW_WIDTH;
    #endif
}

void run_decode_thread(shared_memory_t *shared_memory)
{
    while(decodeRunning){ 
        sem_wait(&shared_memory->sem_decode);
        for(int i = 0;i<NUM_JPEG_BUFFERS;i++){
            if(shared_memory->preview_buffer[0].pre_state == pre_decode && shared_memory->preview_buffer[1].pre_state == pre_decode)printf("%serror?\n",PD);
            if(shared_memory->preview_buffer[i].pre_state == pre_decode){ // wait for buffer to fill
                #ifndef NO_CAM
                    int jpegSubsamp;
                    tjhandle _jpegDecompressor = tjInitDecompress();
                    tjDecompressHeader2(_jpegDecompressor, 
                                        (unsigned char*)shared_memory->preview_buffer[i].jpeg_data, 
                                        shared_memory->preview_buffer[i].size, 
                                        &shared_memory->preview_buffer[i].width, 
                                        &shared_memory->preview_buffer[i].height, 
                                        &jpegSubsamp);
                    //printf("%ssize %dx%d\n",PD,shared_memory->preview_buffer[i].width,shared_memory->preview_buffer[i].height);
                    if(shared_memory->preview_buffer[i].width * shared_memory->preview_buffer[i].height <= PREVIEW_WIDTH * PREVIEW_HEIGHT){               
                        tjDecompress2(  _jpegDecompressor, 
                                        (unsigned char*)shared_memory->preview_buffer[i].jpeg_data, 
                                        shared_memory->preview_buffer[i].size, 
                                        shared_memory->preview_buffer[i].raw_data,
                                        shared_memory->preview_buffer[i].width, 
                                        0,
                                        shared_memory->preview_buffer[i].height, 
                                        TJPF_RGB, TJFLAG_FASTDCT);
                                        shared_memory->preview_buffer[i].pre_state = pre_render;
                                        //printf("%s%d Decode complete\n",PD,i);
                    }else{
                        printf("%sERROR image to large\n",PD);
                        decodeRunning = 0;
                    }
                    tjDestroy(_jpegDecompressor);
                    break;
                #else
                    shared_memory->preview_buffer[i].pre_state = pre_render;
                #endif
            }
        }
        sem_post(&shared_memory->sem_render);
    }
}

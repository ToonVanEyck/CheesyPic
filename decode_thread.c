#include "decode_thread.h"

static int decodeRunning;

void *start_decode_thread(void *shared_buffer)
{
    printf("%sStarted decoding thread!\n",PD);
    init_decode_thread();
    decodeRunning = 1;
    run_decode_thread(shared_buffer);
    printf("%sFinished decoding thread!\n",PD);
}

void stop_decode_thread()
{
    decodeRunning = 0;
}


int init_decode_thread()
{

}

void run_decode_thread(jpeg_buffer_t *shared_buffer)
{
    while(decodeRunning){ 
        if(shared_buffer[0].state == decode){ // wait for buffer to fill
            int jpegSubsamp;
            tjhandle _jpegDecompressor = tjInitDecompress();
            tjDecompressHeader2(_jpegDecompressor, 
                                (unsigned char*)shared_buffer[0].compressed_data, 
                                shared_buffer[0].size, 
                                &shared_buffer[0].width, 
                                &shared_buffer[0].height, 
                                &jpegSubsamp);
            printf("%ssize %dx%d\n",PD,shared_buffer[0].width,shared_buffer[0].height);
            if(shared_buffer[0].width * shared_buffer[0].height <= PREVIEW_WIDTH * PREVIEW_HEIGHT){               
                tjDecompress2(  _jpegDecompressor, 
                                (unsigned char*)shared_buffer[0].compressed_data, 
                                shared_buffer[0].size, 
                                shared_buffer[0].uncompressed_data,
                                shared_buffer[0].width, 
                                0,
                                shared_buffer[0].height, 
                                TJPF_RGB, TJFLAG_FASTDCT);
                                shared_buffer[0].state = render;
                                printf("%sDecode complete\n",PD);
            }else{
                printf("%sERROR image to large\n",PD);
                decodeRunning = 0;
            }
            tjDestroy(_jpegDecompressor);
        }
    }
}

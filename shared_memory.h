#include <semaphore.h> 

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define PREVIEW_WIDTH  1056
#define PREVIEW_HEIGHT 704

#define NUM_JPEG_BUFFERS 2

typedef enum{
    pre_capture,
    pre_decode,
    pre_render
}preview_state_t;

typedef enum{
    log_idle,
    log_triggred,
    log_countdown_3,
    log_countdown_2,
    log_countdown_1,
    log_capture,
    log_preview,
    log_procces,
    log_print
}logic_state_t;

typedef struct{
    unsigned long size;
    void *cameraFile;
    const char *jpeg_data;
    unsigned int width;
    unsigned int height;
    unsigned char raw_data[PREVIEW_WIDTH * PREVIEW_HEIGHT * 3];
    preview_state_t pre_state;
}preview_buffer_t;

typedef struct{

}overlay_bufer;

typedef struct{
    sem_t sem_decode;
    sem_t sem_render;
    preview_buffer_t preview_buffer[NUM_JPEG_BUFFERS];
    //overlay_bufer[] overlay_buffer[2];
    logic_state_t logic_state;
}shared_memory_t;



#endif
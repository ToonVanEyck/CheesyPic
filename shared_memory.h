#include <semaphore.h> 

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define PREVIEW_WIDTH  1056
#define PREVIEW_HEIGHT 704

#define OVERLAY_WIDTH  1620
#define OVERLAY_HEIGHT 1080

#define NUM_JPEG_BUFFERS 2

typedef enum{
    pre_capture,
    pre_decode,
    pre_render
}preview_state_t;

typedef enum{
    log_idle = 0,
    log_triggred,
    log_countdown_3,
    log_countdown_2,
    log_countdown_1,
    log_capture,
    log_preview,
    log_procces,
    log_print
}logic_state_t;
#define FIRST_STATE log_idle
#define LAST_STATE log_print

typedef struct{
    unsigned long size;
    void *cameraFile;
    const char *jpeg_data;
    unsigned int width;
    unsigned int height;
    unsigned char raw_data[PREVIEW_WIDTH * PREVIEW_HEIGHT * 4];
    preview_state_t pre_state;
}preview_buffer_t;

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned char raw_data[OVERLAY_WIDTH * OVERLAY_HEIGHT * 4];
}overlay_bufer_t;

typedef struct{
    sem_t sem_decode;
    sem_t sem_render;
    sem_t sem_logic;
    preview_buffer_t preview_buffer[NUM_JPEG_BUFFERS];
    overlay_bufer_t overlay_buffer;
    logic_state_t logic_state;
}shared_memory_t;



#endif
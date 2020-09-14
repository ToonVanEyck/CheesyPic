#include <semaphore.h> 

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define PREVIEW_WIDTH  1056
#define PREVIEW_HEIGHT 704

#define OVERLAY_WIDTH  1620
#define OVERLAY_HEIGHT 1080

#define CAPTURE_MAX_MP 20

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
    log_decode,
    log_reveal,
    log_procces,
    log_print
}logic_state_t;
#define FIRST_STATE log_idle
#define LAST_STATE log_print

typedef struct{
    unsigned long size;
    void *cameraFile;
    const char *gp_jpeg_data;
    unsigned int width;
    unsigned int height;
    unsigned char raw_data[PREVIEW_WIDTH * PREVIEW_HEIGHT * 4];
    preview_state_t pre_state;
}preview_buffer_t;

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned char raw_data[OVERLAY_WIDTH * OVERLAY_HEIGHT * 4];
}overlay_buffer_t;

typedef struct{
    unsigned long size;
    void *cameraFile;
    const char *gp_jpeg_data;
    unsigned char jpeg_buffer[3000000];
    unsigned int width;
    unsigned int height;
    unsigned char raw_data[CAPTURE_MAX_MP * 4000000];
    int jpeg_copied;
}capture_buffer_t;

typedef struct{
    unsigned char photobooth_active;
    unsigned char fastmode;
    unsigned char preview_mirror;
    unsigned char reveal_mirror;
    unsigned char toggle_printer;
    int camera_error;
    sem_t sem_decode;
    sem_t sem_render;
    sem_t sem_logic;
    preview_buffer_t preview_buffer[NUM_JPEG_BUFFERS];
    overlay_buffer_t overlay_buffer;
    capture_buffer_t capture_buffer;
    logic_state_t logic_state;
}shared_memory_t;



#endif
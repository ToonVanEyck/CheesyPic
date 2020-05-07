#include <semaphore.h> 

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define PREVIEW_WIDTH  1056
#define PREVIEW_HEIGHT 704

#define NUM_JPEG_BUFFERS 2

typedef enum{
    capture,
    decode,
    render
}previeuw_state_t;

typedef struct{
    unsigned long size;
    void *cameraFile;
    const char *compressed_data;
    unsigned int width;
    unsigned int height;
    unsigned char uncompressed_data[PREVIEW_WIDTH * PREVIEW_HEIGHT * 3];
    previeuw_state_t state;
}jpeg_buffer_t;

typedef struct{
    sem_t sem_decode;
    sem_t sem_render;
    jpeg_buffer_t buffer[NUM_JPEG_BUFFERS];
}shared_memory_t;

#endif
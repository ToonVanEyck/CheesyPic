#include "capture_thread.h"

#define FDS_MAX 2
#define FDS_TIMER 0
#define FDS_SIGNAL 1

static int captureRunning;

void start_capture_thread(shared_memory_t *shared_memory)
{
    printf("%sStarted capture thread!\n",PC);
    signal(SIGINT, stop_capture_thread);
    GPContext *ctx;
    Camera *camera;
    struct pollfd fds[FDS_MAX];
    int numfd = 0;
    pthread_t decoder_thread;

    if(init_capture_thread(fds, &numfd, &ctx, &camera)){
        printf("%sFinished capture thread!\n",PC);
        // exit 
        exit(1);
    }else{
        captureRunning = 1;
        if(pthread_create(&decoder_thread, NULL, start_decode_thread, shared_memory)) {
            fprintf(stderr, "Error creating decode thread\n");
            captureRunning = 0;
        }
        run_capture_thread(shared_memory, fds, &numfd, &ctx, &camera);
        sem_post(&shared_memory->sem_decode);
        pthread_join(decoder_thread,NULL);
        clean_capture_thread(fds, &numfd, &ctx, &camera);
    }
    printf("%sFinished capture thread!\n",PC);
}

void stop_capture_thread(int dummy)
{
    captureRunning = 0;
    stop_decode_thread();
}


int init_capture_thread(struct pollfd *fds, int *numfd,GPContext **ctx, Camera **camera)
{
    #ifndef NO_CAM
        if (init_camera(ctx,camera)) return 1;
    #endif 

    memset(fds, 0 , sizeof(fds)); 
    for(int i = 0; i<FDS_MAX ; i++){
        fds[i].fd = -1;
    }

    if (init_timer(fds,numfd)) return 1;
   // if (init_signal(fds,numfd)) return 1;
}

void clean_capture_thread(struct pollfd *fds, int *numfd,GPContext **ctx, Camera **camera)
{
    #ifndef NO_CAM
        gp_camera_exit(*camera,*ctx);
        gp_context_unref(*ctx);
    #endif
    close(fds[FDS_TIMER].fd);
}

int init_timer(struct pollfd *fds, int *numfd)
{
    struct itimerspec timerValue;
    timerValue.it_value.tv_sec = 0;
    timerValue.it_value.tv_nsec = 500000000;
    timerValue.it_interval.tv_sec = SLOWDOWN;
    timerValue.it_interval.tv_nsec = FRAME_PERIOD; 

    fds[FDS_TIMER].fd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (fds[FDS_TIMER].fd == -1) {
        fprintf(stderr, "Failed to create timerfd\n");
        return 1;
    }
    if (timerfd_settime(fds[FDS_TIMER].fd, 0, &timerValue, NULL) < 0) {
        fprintf(stderr, "could not start timer\n");
        return 1;
    }
    fds[FDS_TIMER].events = POLLIN;
    (*numfd)++;
    return 0;
}

int init_signal(struct pollfd *fds, int *numfd)
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);

    fds[FDS_SIGNAL].fd = signalfd(-1, &sigset, 0);

    if(fds[FDS_SIGNAL].fd == -1){
        fprintf(stderr, "Failed to allocate signalfd\n");
        return 1;
    }
    if (pthread_sigmask(SIG_BLOCK, &sigset, NULL) == -1){
        fprintf(stderr, "Failed to block signals\n");
        return 1;
    }
    fds[FDS_SIGNAL].events = POLLIN;
    (*numfd)++;
    return 0;
}


int init_camera(GPContext **ctx, Camera **camera)
{
    *ctx = gp_context_new();
    CameraList *cameraList;
	gp_list_new (&cameraList);
    int count = gp_camera_autodetect(cameraList, *ctx);
    if (count <= 0){
        if (count == 0) {
            printf("%sNo cameras detected\n",PC);
        } else {
            printf("%sGP ERROR :%d\n",PC,count);
        }
        gp_context_unref(*ctx);
        return 1;
    }else{
		printf("%scameras detected\n",PC);
		const char *modelName = NULL, *portName = NULL;
		gp_list_get_name  (cameraList, 0, &modelName);
		gp_list_get_value (cameraList, 0, &portName);
		printf("%sfound model: %s @ %s\n",PC,modelName, portName);
		CameraAbilitiesList *al = NULL;
		CameraAbilities a;
		gp_abilities_list_new (&al);
		gp_abilities_list_load (al, NULL);
		int i = gp_abilities_list_lookup_model (al, modelName);
		if (i < 0)
			printf("%sCould not find model: '%s'.\n",PC,gp_result_as_string (i));
		gp_abilities_list_get_abilities (al, i, &a);
		GPPortInfoList *il = NULL;
		GPPortInfo info;
		gp_port_info_list_new (&il);
		gp_port_info_list_load (il);
		i = gp_port_info_list_lookup_path (il, portName);
		if (i < 0)
			printf("%sCould not find port: '%s'.\n",PC,gp_result_as_string (i));
		gp_port_info_list_get_info (il, i, &info);

		/* Capture an image, download it and delete it. */
		printf("%sInitializing camera...\n",PC);
		CameraFilePath path;
		const char *data;
        const char *mime_type;
		unsigned long size;

		i = gp_camera_new (camera);
		printf("%s%d %s\n",PC,i,a.model);
		gp_camera_set_abilities (*camera, a);
		gp_camera_set_port_info (*camera, info);

        gp_list_free(cameraList);
		gp_abilities_list_free (al);
		gp_port_info_list_free (il);

        CameraFile *cf;
        gp_file_new(&cf);
        gp_camera_capture_preview (*camera, cf, *ctx);
        gp_file_free(cf);
        
        return 0;
    }
}

static void print_fps(void)
{
    static int frames = 0;
    static double tRate0 = -1.0;

    struct timeval tv;
    gettimeofday(&tv, NULL );
    double t = tv.tv_sec + tv.tv_usec / 1000000.0; 
    frames++;
    if (tRate0 < 0.0)
        tRate0 = t;
    if (t - tRate0 >= 5.0) {
        float seconds = t - tRate0;
        float fps = frames / seconds;
        printf("\33[0m%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds,fps);
        tRate0 = t;
        frames = 0;
    }
}


void run_capture_thread(shared_memory_t *shared_memory, struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera)
{
    while(captureRunning){
        poll(fds,*numfd,-1);
        if(fds[FDS_TIMER].revents & POLLIN){
            uint64_t value;
            read(fds[FDS_TIMER].fd, &value,8);  
            for(int i = 0;i<NUM_JPEG_BUFFERS;i++){
                // clear previous frame
                if(shared_memory->preview_buffer[i].pre_state != pre_decode && shared_memory->preview_buffer[i].cameraFile){
                    #ifndef NO_CAM
                        gp_file_free((CameraFile *)shared_memory->preview_buffer[i].cameraFile); // only free after decode!
                    #endif
                    shared_memory->preview_buffer[i].cameraFile = NULL;
                }
                // // get new frame
                #ifdef NO_CAM
                    static unsigned char prev_buffer=0;
                    if(prev_buffer == i){
                        continue;
                    }else{
                        prev_buffer = i;
                    }
                #endif

                if(shared_memory->preview_buffer[i].pre_state == pre_capture){
                    #ifndef NO_CAM
                        int rc[3];
                        rc[0] = gp_file_new ((CameraFile **)&shared_memory->preview_buffer[i].cameraFile);
                        rc[1] = gp_camera_capture_preview (*camera, (CameraFile *)shared_memory->preview_buffer[i].cameraFile, *ctx);
                        rc[2] = gp_file_get_data_and_size ((CameraFile *)shared_memory->preview_buffer[i].cameraFile, &(shared_memory->preview_buffer[i].jpeg_data),&(shared_memory->preview_buffer[i].size));
                        if(rc[0]||rc[0]||rc[0]){
                            printf("%s[%d] [%d] [%d]\n",PC,rc[0],rc[1],rc[2]);
                        }
                    #endif
                    shared_memory->preview_buffer[i].pre_state = pre_decode;
                    sem_post(&shared_memory->sem_decode);
                    //printf("%s%d Capture complete\n",PC,i);
                    print_fps();
                    break;
                }else{
                //     captureRunning = 0;
                }
            }
        }
    }
}


#include "capture_thread.h"

#define FDS_MAX 2
#define FDS_TIMER 0
#define FDS_SIGNAL 1

static int keepRunning;

void start_capture_thread(void *shmem)
{
    GPContext *ctx;
    Camera *camera;
    struct pollfd fds[FDS_MAX];
    int numfd = 0;

    if(init_capture_thread(fds, &numfd, &ctx, &camera)){
        // exit 
        exit(1);
    }else{
        keepRunning = 1;
    

        run_capture_thread(shmem, fds, &numfd, &ctx, &camera);
        gp_camera_exit(camera,ctx);
        gp_context_unref(ctx);
    }
}

void stop_capture_thread()
{
    keepRunning = 0;
}


int init_capture_thread(struct pollfd *fds, int *numfd,GPContext **ctx, Camera **camera)
{
    // if (init_camera(ctx,camera)) return 1;

    memset(fds, 0 , sizeof(fds)); 
    for(int i = 0; i<FDS_MAX ; i++){
        fds[i].fd = -1;
    }

    if (init_timer(fds,numfd)) return 1;
    if (init_signal(fds,numfd)) return 1;
}

int init_timer(struct pollfd *fds, int *numfd)
{
    struct itimerspec timerValue;
    timerValue.it_value.tv_sec = 0;
    timerValue.it_value.tv_nsec = 40000000;
    timerValue.it_interval.tv_sec = 0;
    timerValue.it_interval.tv_nsec = 40000000;  // 25fps

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
    if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1){
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
            printf("No cameras detected\n");
        } else {
            printf("GP ERROR :%d\n",count);
        }
        gp_context_unref(*ctx);
        return 1;
    }else{
		printf("cameras detected\n");
		const char *modelName = NULL, *portName = NULL;
		gp_list_get_name  (cameraList, 0, &modelName);
		gp_list_get_value (cameraList, 0, &portName);
		printf("found model: %s @ %s\n",modelName, portName);
		CameraAbilitiesList *al = NULL;
		CameraAbilities a;
		gp_abilities_list_new (&al);
		gp_abilities_list_load (al, NULL);
		int i = gp_abilities_list_lookup_model (al, modelName);
		if (i < 0)
			printf("Could not find model: '%s'.\n",gp_result_as_string (i));
		gp_abilities_list_get_abilities (al, i, &a);
		GPPortInfoList *il = NULL;
		GPPortInfo info;
		gp_port_info_list_new (&il);
		gp_port_info_list_load (il);
		i = gp_port_info_list_lookup_path (il, portName);
		if (i < 0)
			printf("Could not find port: '%s'.\n",gp_result_as_string (i));
		gp_port_info_list_get_info (il, i, &info);

		/* Capture an image, download it and delete it. */
		printf("Initializing camera...\n");
		CameraFilePath path;
		const char *data;
        const char *mime_type;
		unsigned long size;

		i = gp_camera_new (camera);
		printf("%d %s\n",i,a.model);
		gp_camera_set_abilities (*camera, a);
		gp_camera_set_port_info (*camera, info);

        gp_list_free(cameraList);
		gp_abilities_list_free (al);
		gp_port_info_list_free (il);
        return 0;
    }
}

static double current_time(void)
{
   struct timeval tv;
   gettimeofday(&tv, NULL );
   return (double) tv.tv_sec + tv.tv_usec / 1000000.0;
}


void run_capture_thread(void *shmem,struct pollfd *fds, int *numfd, GPContext **ctx, Camera **camera)
{
    while(keepRunning){
        poll(fds,*numfd,-1);
        if(fds[FDS_TIMER].revents & POLLIN){
            uint64_t value;
            read(fds[FDS_TIMER].fd, &value,8);  
            // check fps
            // get frame
            static int frames = 0;
            static double tRate0 = -1.0;
            double t = current_time();
            frames++;
            if (tRate0 < 0.0)
                tRate0 = t;
            if (t - tRate0 >= 5.0) {
                float seconds = t - tRate0;
                float fps = frames / seconds;
                sprintf(&((char*)shmem)[1],"%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds,fps);
                ((char*)shmem)[0]=1;
                tRate0 = t;
                frames = 0;
            }
        }
        if(fds[FDS_SIGNAL].revents & POLLIN){
            struct signalfd_siginfo info;
            size_t bytes = read(fds[FDS_SIGNAL].fd, &info, sizeof(info));
            if (bytes == sizeof(info)){
                unsigned sig = info.ssi_signo;
                if (sig == SIGINT){
                    printf("Got SIGINT: Server will teminate!\n");
                    keepRunning = 0;
                }
            }
        }
    }
}

